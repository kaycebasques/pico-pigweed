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

#include "ft6236/device.h"

#include <chrono>
#include <cstddef>
#include <cstdint>

#define PW_LOG_MODULE_NAME "ft6236"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "pw_bytes/bit.h"
#include "pw_bytes/endian.h"
#include "pw_i2c/address.h"
#include "pw_i2c/register_device.h"
#include "pw_log/log.h"
#include "pw_status/status.h"

using namespace std::chrono_literals;
using pw::Status;
using pw::bytes::ReadInOrder;

namespace pw::ft6236 {

namespace {

enum Ft62xxRegister : uint8_t {
  kThreshhold = 0x80,
  kPointrate = 0x88,
  kChipid = 0xA3,
  kFirmvers = 0xA6,
  kVendid = 0xA8,
};

constexpr pw::i2c::Address kAddress = pw::i2c::Address::SevenBit<0x38>();

}  // namespace

Device::Device(pw::i2c::Initiator& initiator)
    : initiator_(initiator),
      device_(initiator,
              kAddress,
              endian::little,
              pw::i2c::RegisterAddressSize::k1Byte) {}
Device::~Device() = default;

Status Device::Enable() {
  Result<std::byte> vendor_id_result = device_.ReadRegister(
      Ft62xxRegister::kVendid, pw::chrono::SystemClock::for_at_least(10ms));
  if (vendor_id_result.value_or(std::byte{0}) != std::byte{0x11}) {
    return Status::NotFound();
  }

  SetThreshhold(128);

  return OkStatus();
}

Status Device::SetThreshhold(uint8_t threshhold) {
  PW_TRY(device_.WriteRegister(Ft62xxRegister::kThreshhold,
                               std::byte{threshhold},
                               pw::chrono::SystemClock::for_at_least(10ms)));
  return OkStatus();
}

Status Device::Probe() {
  pw::Status probe_result(initiator_.ProbeDeviceFor(
      kAddress, pw::chrono::SystemClock::for_at_least(10ms)));

  if (probe_result != pw::OkStatus()) {
    PW_LOG_DEBUG("FT6236 Probe Failed");
  } else {
    PW_LOG_DEBUG("FT6236 Probe Ok");
  }
  return probe_result;
}

void Device::LogControllerInfo() {
  Result<std::byte> result = device_.ReadRegister(
      Ft62xxRegister::kVendid, pw::chrono::SystemClock::for_at_least(10ms));
  PW_LOG_DEBUG("Vend ID: 0x%x", (uint8_t)result.value_or(std::byte{0}));

  result = device_.ReadRegister(Ft62xxRegister::kChipid,
                                pw::chrono::SystemClock::for_at_least(10ms));
  PW_LOG_DEBUG("Chip ID: 0x%x (0x36==FT6236)",
               (uint8_t)result.value_or(std::byte{0}));

  result = device_.ReadRegister(Ft62xxRegister::kFirmvers,
                                pw::chrono::SystemClock::for_at_least(10ms));
  PW_LOG_DEBUG("Firmware Version: %u", (uint8_t)result.value_or(std::byte{0}));

  result = device_.ReadRegister(Ft62xxRegister::kPointrate,
                                pw::chrono::SystemClock::for_at_least(10ms));
  PW_LOG_DEBUG("Point Rate Hz: %u", (uint8_t)result.value_or(std::byte{0}));

  result = device_.ReadRegister(Ft62xxRegister::kThreshhold,
                                pw::chrono::SystemClock::for_at_least(10ms));
  PW_LOG_DEBUG("Threshhold: %u", (uint8_t)result.value_or(std::byte{0}));
}

void Device::LogTouchInfo() {
  if (touch_count_ == 0) {
    return;
  }

  for (int i = 0; i < touch_count_; i++) {
    PW_LOG_DEBUG("Touch%d: (x,y)=(%d, %d) weight=%d area=%d",
                 i + 1,
                 touches_[i].x,
                 touches_[i].y,
                 touches_[i].weight,
                 touches_[i].area);
  }
}

int Device::TouchCount() { return touch_count_; }

pw::ft6236::Touch Device::Touch1() { return touches_[0]; }

pw::ft6236::Touch Device::Touch2() { return touches_[1]; }

Result<bool> Device::ReadData() {
  // Read 16 registers starting from 0x00.
  std::array<std::byte, 16> rx_buffer;
  PW_TRY(device_.ReadRegisters(
      0, rx_buffer, pw::chrono::SystemClock::for_at_least(10ms)));

  // Number of touches (0, 1 or 2) is at 0x02.
  touch_count_ = ReadInOrder<uint8_t>(endian::big, &rx_buffer[0x02]);

  // Return false if no new touches are present.
  if (touch_count_ == 0) {
    return false;
  }

  // Read Touch #1 X coordinate high (0x03) and low (0x04) registers.
  touches_[0].x = ReadInOrder<uint16_t>(endian::big, &rx_buffer[0x03]) & 0xFFF;
  // Read Touch #1 Y coordinate high (0x05) and low (0x06) registers.
  touches_[0].y = ReadInOrder<uint16_t>(endian::big, &rx_buffer[0x05]) & 0xFFF;

  // Read Touch #1 Misc data
  touches_[0].weight = ReadInOrder<uint8_t>(endian::big, &rx_buffer[0x07]);
  touches_[0].area = ReadInOrder<uint8_t>(endian::big, &rx_buffer[0x08]) & 0x0F;

  // Read Touch #2 X coordinate high (0x09) and low (0x0A) registers.
  touches_[1].x = ReadInOrder<uint16_t>(endian::big, &rx_buffer[0x09]) & 0xFFF;
  // Read Touch #2 Y coordinate high (0x0B) and low (0x0C) registers.
  touches_[1].y = ReadInOrder<uint16_t>(endian::big, &rx_buffer[0x0B]) & 0xFFF;

  // Read Touch #2 Misc data
  touches_[1].weight = ReadInOrder<uint8_t>(endian::big, &rx_buffer[0x0D]);
  touches_[1].area = ReadInOrder<uint8_t>(endian::big, &rx_buffer[0x0E]) & 0x0F;

  return true;
}

}  // namespace pw::ft6236
