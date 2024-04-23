// Copyright 2023 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#include "icm42670p/device.h"

#include <chrono>
#include <cstddef>
#include <cstdint>

#define PW_LOG_MODULE_NAME "icm42670p"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "pw_bytes/bit.h"
#include "pw_i2c/address.h"
#include "pw_i2c/register_device.h"
#include "pw_log/log.h"
#include "pw_result/result.h"
#include "pw_status/status.h"

using ::pw::Status;
using namespace std::chrono_literals;

namespace kudzu::icm42670p {

enum Icm42670pRegister : uint8_t {
  kWhoAmI = 0x75,
  kPwrMgmt0 = 0x1f,
  kGyroConfig0 = 0x20,
  kAccelConfig0 = 0x21,
  kAccelDataX1 = 0x0b,
  kGyroDataX1 = 0x11,
};

constexpr int16_t GYRO_UI_FS_SEL = 1000;
constexpr float ACCEL_UI_FS_SEL = 8.0f;

namespace {

constexpr pw::i2c::Address kAddress = pw::i2c::Address::SevenBit<0x68>();

void ImuReadReg(pw::i2c::RegisterDevice& device,
                uint8_t addr,
                const char* name) {
  auto data =
      device.ReadRegister8(addr, pw::chrono::SystemClock::for_at_least(10ms));

  if (data.ok()) {
    PW_LOG_INFO("%s: %02x", name, *data);
  } else {
    PW_LOG_INFO("failed to read %s", name);
  }
}

void ImuReadReg16(pw::i2c::RegisterDevice& device,
                  uint8_t addr,
                  const char* name) {
  auto data =
      device.ReadRegister16(addr, pw::chrono::SystemClock::for_at_least(10ms));

  if (data.ok()) {
    PW_LOG_INFO("%s: %04x", name, *data);
  } else {
    PW_LOG_INFO("failed to read %s", name);
  }
}

pw::Result<kudzu::imu::ImuSample> ReadData(pw::i2c::RegisterDevice& device) {
  // TODO: asadmemon - Check the status register before reading the values.

  uint8_t data[12];
  device.ReadRegisters8(Icm42670pRegister::kAccelDataX1,
                        pw::span(data, 12),
                        pw::chrono::SystemClock::for_at_least(10ms));

  uint16_t accel_x =
      pw::bytes::ReadInOrder<uint16_t>(pw::endian::big, &data[0]);
  uint16_t accel_y =
      pw::bytes::ReadInOrder<uint16_t>(pw::endian::big, &data[2]);
  uint16_t accel_z =
      pw::bytes::ReadInOrder<uint16_t>(pw::endian::big, &data[4]);

  uint16_t gyro_x = pw::bytes::ReadInOrder<uint16_t>(pw::endian::big, &data[6]);
  uint16_t gyro_y = pw::bytes::ReadInOrder<uint16_t>(pw::endian::big, &data[8]);
  uint16_t gyro_z =
      pw::bytes::ReadInOrder<uint16_t>(pw::endian::big, &data[10]);

  float f_accel_x =
      static_cast<float>((int16_t)accel_x) * ACCEL_UI_FS_SEL / INT16_MAX;
  float f_accel_y =
      static_cast<float>((int16_t)accel_y) * ACCEL_UI_FS_SEL / INT16_MAX;
  float f_accel_z =
      static_cast<float>((int16_t)accel_z) * ACCEL_UI_FS_SEL / INT16_MAX;

  int16_t f_gyro_x =
      static_cast<int16_t>((int16_t)gyro_x) * GYRO_UI_FS_SEL / INT16_MAX;
  int16_t f_gyro_y =
      static_cast<int16_t>((int16_t)gyro_y) * GYRO_UI_FS_SEL / INT16_MAX;
  int16_t f_gyro_z =
      static_cast<int16_t>((int16_t)gyro_z) * GYRO_UI_FS_SEL / INT16_MAX;

  kudzu::imu::ImuSample sample = {
      {f_accel_x, f_accel_y, f_accel_z},
      {f_gyro_x, f_gyro_y, f_gyro_z},
  };
  return sample;
}

}  // namespace

Device::Device(pw::i2c::Initiator& initiator)
    : initiator_(initiator),
      device_(initiator,
              kAddress,
              pw::endian::little,
              pw::i2c::RegisterAddressSize::k1Byte) {}

pw::Status Device::Enable() {
  device_.WriteRegister8(
      0x1f, 0x0f, pw::chrono::SystemClock::for_at_least(10ms));

  /*
  +----------------+-----------------------------------+
  | ACCEL_CONFIG0  |                                   |
  +----------------+-----------------------------------+
  | Name           | ACCEL_CONFIG0                     |
  | Address        | 33 (21h)                          |
  | Serial IF      | R/W                               |
  | Reset value    | 0x06                              |
  +----------------+-----------------------------------+
  | BIT  | NAME            | FUNCTION                  |
  +----------------+-----------------------------------+
  | 7    | -               | Reserved                  |
  | 6:5  | ACCEL_UI_FS_SEL | 00: ±16g                  |
  |      |                 | 01: ±8g                   |
  |      |                 | 10: ±4g                   |
  |      |                 | 11: ±2g                   |
  | 4    | -               | Reserved                  |
  | 3:0  | ACCEL_ODR       | 0101: 1.6k Hz (LN mode)   |
  |      |                 | 0110: 800 Hz (LN mode)    |
  |      |                 | 0111: 400 Hz (LP or LN)   |
  |      |                 | 1000: 200 Hz (LP or LN)   |
  |      |                 | 1001: 100 Hz (LP or LN)   |
  |      |                 | 1010: 50 Hz (LP or LN)    |
  |      |                 | 1011: 25 Hz (LP or LN)    |
  |      |                 | 1100: 12.5 Hz (LP or LN)  |
  |      |                 | 1101: 6.25 Hz (LP mode)   |
  |      |                 | 1110: 3.125 Hz (LP mode)  |
  |      |                 | 1111: 1.5625 Hz (LP mode) |
  +----------------+-----------------------------------+
  */
  uint8_t new_accel_config = 0b00101000;
  device_.WriteRegister8(Icm42670pRegister::kAccelConfig0,
                         new_accel_config,
                         pw::chrono::SystemClock::for_at_least(10ms));

  /*
  +--------------+------------------------+
  | GYRO_CONFIG0 |                        |
  +--------------+------------------------+
  | Name         | GYRO_CONFIG0           |
  | Address      | 32 (20h)               |
  | Serial IF    | R/W                    |
  | Reset value  | 0x06                   |
  +--------------+------------------------+
  | BIT  | NAME           | FUNCTION      |
  +--------------+------------------------+
  | 7    | -              | Reserved      |
  | 6:5  | GYRO_UI_FS_SEL | 00: ±2000 dps |
  |      |                | 01: ±1000 dps |
  |      |                | 10: ±500 dps  |
  |      |                | 11: ±250 dps  |
  | 4    | -              | Reserved      |
  | 3:0  | GYRO_ODR       | 0101: 1.6k Hz |
  |      |                | 0110: 800 Hz  |
  |      |                | 0111: 400 Hz  |
  |      |                | 1000: 200 Hz  |
  |      |                | 1001: 100 Hz  |
  |      |                | 1010: 50 Hz   |
  |      |                | 1011: 25 Hz   |
  |      |                | 1100: 12.5 Hz |
  +--------------+------------------------+
  */
  uint8_t new_gyro_config = 0b00101000;
  device_.WriteRegister8(Icm42670pRegister::kGyroConfig0,
                         new_gyro_config,
                         pw::chrono::SystemClock::for_at_least(10ms));

  return pw::OkStatus();
}

Status Device::Probe() {
  pw::Status probe_result(initiator_.ProbeDeviceFor(
      kAddress, pw::chrono::SystemClock::for_at_least(50ms)));

  if (probe_result != pw::OkStatus()) {
    PW_LOG_DEBUG("ICM-42670-P Probe Failed");
  } else {
    PW_LOG_DEBUG("ICM-42670-P Probe Ok");
  }
  return probe_result;
}

void Device::LogControllerInfo() {
  device_.WriteRegister8(
      0x1f, 0x0f, pw::chrono::SystemClock::for_at_least(10ms));

  ImuReadReg(device_, Icm42670pRegister::kWhoAmI, "WHO_AM_I");
  ImuReadReg(device_, Icm42670pRegister::kPwrMgmt0, "PWR_MGMT0");
  ImuReadReg(device_, Icm42670pRegister::kGyroConfig0, "GYRO_CONFIG0");
  ImuReadReg(device_, Icm42670pRegister::kAccelConfig0, "ACCEL_CONFIG0");
}

pw::Result<kudzu::imu::ImuSample> Device::ReadValues() {
  return ReadData(device_);
}

}  // namespace kudzu::icm42670p
