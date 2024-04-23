// Copyright 2024 The Pigweed Authors
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

#include <bitset>
#include <chrono>
#include <cstdint>

#include "pw_chrono/system_clock.h"
#include "pw_result/result.h"
#include "pw_status/status.h"

namespace kudzu::button {

enum ButtonName {
  up = 0,
  right = 1,
  left = 2,
  down = 3,
  select = 4,
  start = 5,
  b = 6,
  a = 7,
};

}  // namespace kudzu::button

namespace kudzu {

constexpr int kButtonCount = 8;

class Buttons {
 public:
  virtual ~Buttons() = default;
  virtual pw::Status Init() = 0;
  /// Fetch the latest button states and update held times.
  pw::Status Update();

  /// Function to be implemented that returns a bitset of length kButtonCount. A
  /// button press corresponds to a bit == 1 and a release == 0.
  virtual pw::Result<std::bitset<kButtonCount>> DoUpdate() = 0;

  /// Returns true if the button was just pressed. This only fires once on the
  /// transition from not pressed to pressed.
  inline bool Pressed(kudzu::button::ButtonName button_name) {
    return button_bits_[button_name] && !button_bits_previous_[button_name];
  }

  /// Returns true if the button was just released. This only fires once on the
  /// transition from held to released.
  inline bool Released(kudzu::button::ButtonName button_name) {
    return !button_bits_[button_name] && button_bits_previous_[button_name];
  }

  /// Returns true if the button is being held down.
  inline bool Held(kudzu::button::ButtonName button_name) {
    return button_bits_[button_name] && button_bits_previous_[button_name];
  }

  /// Button hold duration accessor.
  ///
  /// @code
  ///   #include "pw_chrono/system_clock.h"
  ///   using namespace std::chrono_literals;
  ///
  ///   if (buttons.HeldDuration(kudzu::button::up) >
  ///       pw::chrono::SystemClock::for_at_least(1000ms)) {
  ///     PW_LOG_INFO("Up button held for one second.");
  ///   }
  /// @endcode
  inline pw::chrono::SystemClock::duration HeldDuration(
      kudzu::button::ButtonName button_name) {
    return button_hold_duration_[button_name];
  }

 private:
  pw::chrono::SystemClock::time_point update_time_previous_;
  pw::chrono::SystemClock::time_point update_time_;
  std::bitset<kButtonCount> button_bits_;
  std::bitset<kButtonCount> button_bits_previous_;
  std::array<pw::chrono::SystemClock::duration, kButtonCount>
      button_hold_duration_;
};

}  // namespace kudzu
