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

#include "pi4ioe5v6416/device.h"

#include <bitset>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

#define PW_LOG_MODULE_NAME "pi4ioe5v6416"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "pw_bytes/bit.h"
#include "pw_i2c/address.h"
#include "pw_i2c/register_device.h"
#include "pw_log/log.h"
#include "pw_status/status.h"

using ::pw::Status;
using namespace std::chrono_literals;

namespace pw::pi4ioe5v6416 {

namespace {

constexpr pw::i2c::Address kAddress = pw::i2c::Address::SevenBit<0x20>();
enum Register : uint32_t {
  InputPort0 = 0x0,
  InputPort1 = 0x1,
  OutputPort0 = 0x2,
  OutputPort1 = 0x3,
  ConfigPort0 = 0x6,
  ConfigPort1 = 0x7,
  PullUpDownEnablePort0 = 0x46,
  PullUpDownEnablePort1 = 0x47,
  PullUpDownSelectionPort0 = 0x48,
  PullUpDownSelectionPort1 = 0x49,
};

}  // namespace

Device::Device(pw::i2c::Initiator& initiator)
    : initiator_(initiator),
      device_(initiator,
              kAddress,
              endian::little,
              pw::i2c::RegisterAddressSize::k1Byte) {}

Status Device::Enable() {
  // Set port 0 as inputs for buttons: 1=input 0=output
  device_.WriteRegister8(
      Register::ConfigPort0, 0xff, pw::chrono::SystemClock::for_at_least(10ms));
  // Select pull up resistors for button input: 1=pull-up 0=pull-down.
  device_.WriteRegister8(Register::PullUpDownSelectionPort0,
                         0xff,
                         pw::chrono::SystemClock::for_at_least(10ms));
  // Enable pull up/down resistors for button input: 1=enable 0=disable.
  device_.WriteRegister8(Register::PullUpDownEnablePort0,
                         0xff,
                         pw::chrono::SystemClock::for_at_least(10ms));

  // Port 1 pins 6 and 7 are DISP_RESET and TOUCH_RESET
  // Set port 1 pins 6 and 7 to outputs: 1=input 0=output
  device_.WriteRegister8(Register::ConfigPort1,
                         0b00111111,
                         pw::chrono::SystemClock::for_at_least(10ms));
  // Set pins 6 and 7 to high.
  device_.WriteRegister8(
      Register::OutputPort1, 0xff, pw::chrono::SystemClock::for_at_least(10ms));

  return OkStatus();
}

pw::Result<uint8_t> Device::ReadPort0() {
  return device_.ReadRegister8(Register::InputPort0,
                               pw::chrono::SystemClock::for_at_least(10ms));
}

pw::Result<uint8_t> Device::ReadPort1() {
  return device_.ReadRegister8(Register::InputPort1,
                               pw::chrono::SystemClock::for_at_least(10ms));
}

Status Device::Probe() {
  pw::Status probe_result(initiator_.ProbeDeviceFor(
      kAddress, pw::chrono::SystemClock::for_at_least(10ms)));

  if (probe_result.ok()) {
    PW_LOG_DEBUG("PI4IOE5V6416 Probe Ok");
  } else {
    PW_LOG_ERROR("PI4IOE5V6416 Probe Failed");
  }
  return probe_result;
}

void Device::LogControllerInfo() {
  pw::Result<uint8_t> port0_result = ReadPort0();
  if (port0_result.ok()) {
    PW_LOG_INFO("Port 0: %02x", port0_result.value());
    std::bitset<8> button(port0_result.value());
    std::string button_str = button.to_string();
    PW_LOG_INFO("Buttons: %s", button_str.data());
  } else {
    PW_LOG_ERROR("Port 0 read failed");
  }

  pw::Result<uint8_t> port1_result = ReadPort1();
  if (port1_result.ok()) {
    PW_LOG_INFO("Port 1: %02x", port1_result.value());
  } else {
    PW_LOG_ERROR("Port 1 read failed");
  }
}

}  // namespace pw::pi4ioe5v6416
