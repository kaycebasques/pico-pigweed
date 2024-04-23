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

#include "kudzu_buttons/buttons.h"

#include <chrono>
#include <cstdint>

#define PW_LOG_MODULE_NAME "kudzu_buttons"
#define PW_LOG_LEVEL PW_LOG_LEVEL_INFO

#include "pw_log/log.h"

namespace kudzu {

pw::Status Buttons::Update() {
  // Get the new button bits.
  pw::Result<std::bitset<kButtonCount>> update_result = DoUpdate();
  if (!update_result.ok()) {
    return update_result.status();
  }

  update_time_previous_ = update_time_;
  update_time_ = pw::chrono::SystemClock::now();
  pw::chrono::SystemClock::duration update_delta =
      update_time_ - update_time_previous_;

  button_bits_previous_ = button_bits_;
  button_bits_ = update_result.value();

  // Log if buttons changed.
  if (button_bits_previous_ != button_bits_) {
    std::string button_str = button_bits_.to_string();
    PW_LOG_DEBUG("Buttons: %s", button_str.data());
  }

  // Track how long each button has been held.
  for (int i = 0; i < kButtonCount; i++) {
    if (Pressed(kudzu::button::ButtonName(i))) {
      button_hold_duration_[i] = pw::chrono::SystemClock::duration(0);
      PW_LOG_DEBUG("Button %d pressed", i);
    } else if (Released(kudzu::button::ButtonName(i))) {
      button_hold_duration_[i] = pw::chrono::SystemClock::duration(0);
      PW_LOG_DEBUG("Button %d released", i);
    } else if (Held(kudzu::button::ButtonName(i))) {
      button_hold_duration_[i] += update_delta;
    }
  }

  return pw::OkStatus();
};

}  // namespace kudzu
