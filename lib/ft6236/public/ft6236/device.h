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
#pragma once

#include <array>
#include <cstdint>

#include "pw_i2c/address.h"
#include "pw_i2c/initiator.h"
#include "pw_i2c/register_device.h"
#include "pw_status/status.h"

namespace pw::ft6236 {

// Struct to store a single touch event from the FT6236.
struct Touch {
  // X coordinate in pixels
  uint16_t x;
  // Y coordinate in pixels
  uint16_t y;
  // Touch pressure value
  uint8_t weight;
  // Touch area value
  uint8_t area;
};

class Device {
 public:
  Device(pw::i2c::Initiator& initiator);
  ~Device();

  // Check the FT6236 is readable and set an initial threshhold value.
  Status Enable();
  // Probe the I2C bus for the FT6236 address (0x38) and return the result.
  Status Probe();
  // Log various registers for debugging purposes.
  void LogControllerInfo();
  // Log the last read touch data.
  void LogTouchInfo();

  // Set the touch detection threshold value.
  Status SetThreshhold(uint8_t threshhold);
  // Read touch data from the FT6236 and store locally.
  // Returns true if new touch events are found, false otherwise.
  Result<bool> ReadData();

  // Return the number of touches from the last update.
  int TouchCount();
  // Return Touch #1 values from the last update.
  Touch Touch1();
  // Return Touch #2 values from the last update.
  Touch Touch2();

 private:
  pw::i2c::Initiator& initiator_;
  pw::i2c::RegisterDevice device_;

  std::array<Touch, 2> touches_;
  uint8_t touch_count_;
};

}  // namespace pw::ft6236
