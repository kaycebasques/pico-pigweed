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

#include "max17048/device.h"

#include <chrono>
#include <cstddef>
#include <cstdint>

#define PW_LOG_MODULE_NAME "max17048"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "pw_bytes/bit.h"
#include "pw_i2c/address.h"
#include "pw_i2c/register_device.h"
#include "pw_log/log.h"
#include "pw_status/status.h"

using ::pw::Status;
using namespace std::chrono_literals;

namespace pw::max17048 {

namespace {

constexpr pw::i2c::Address kAddress = pw::i2c::Address::SevenBit<0x36>();

void FuelReadReg(pw::i2c::RegisterDevice& device,
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

}  // namespace

Device::Device(pw::i2c::Initiator& initiator)
    : initiator_(initiator),
      device_(initiator,
              kAddress,
              endian::little,
              pw::i2c::RegisterAddressSize::k1Byte) {}

Status Device::Enable() {
  device_.WriteRegister8(
      0x1f, 0x0f, pw::chrono::SystemClock::for_at_least(10ms));

  return OkStatus();
}

Status Device::Probe() {
  pw::Status probe_result(initiator_.ProbeDeviceFor(
      kAddress, pw::chrono::SystemClock::for_at_least(10ms)));

  if (probe_result != pw::OkStatus()) {
    PW_LOG_DEBUG("MAX17048 Probe Failed");
  } else {
    PW_LOG_DEBUG("MAX17048 Probe Ok");
  }
  return probe_result;
}

void Device::LogControllerInfo() {
  auto data =
      device_.ReadRegister16(0x2, pw::chrono::SystemClock::for_at_least(10ms));
  if (data.ok()) {
    PW_LOG_INFO("VCELL: %d mV", static_cast<uint32_t>(*data) * 78125 / 1000000);
  } else {
    PW_LOG_INFO("failed to read VCELL");
  }
  FuelReadReg(device_, 0x4, "SOC");
  FuelReadReg(device_, 0x6, "MODE");
  FuelReadReg(device_, 0x8, "VERSION");
  FuelReadReg(device_, 0xa, "HIBRT");
  FuelReadReg(device_, 0xc, "CONFIG");
  FuelReadReg(device_, 0x14, "VALRT");
  FuelReadReg(device_, 0x16, "CRATE");
  FuelReadReg(device_, 0x18, "VRESET/ID");
  FuelReadReg(device_, 0x1a, "STATUS");
}

}  // namespace pw::max17048
