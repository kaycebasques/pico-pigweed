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

#include <bitset>

#define PW_LOG_MODULE_NAME "kudzu_buttons_pi4ioe5v6416"
#define PW_LOG_LEVEL PW_LOG_LEVEL_INFO

#include "kudzu_buttons_pi4ioe5v6416/buttons.h"
#include "pi4ioe5v6416/device.h"
#include "pw_log/log.h"

namespace kudzu {

ButtonsPI4IOE5V6416::ButtonsPI4IOE5V6416(pw::pi4ioe5v6416::Device* controller)
    : controller_(controller) {}

pw::Status ButtonsPI4IOE5V6416::Init() {
  controller_->Enable();
  return pw::OkStatus();
}

pw::Result<std::bitset<kButtonCount>> ButtonsPI4IOE5V6416::DoUpdate() {
  pw::Result<uint8_t> result = controller_->ReadPort0();
  if (!result.ok()) {
    return result.status();
  }
  std::bitset<kButtonCount> new_bits = result.value();
  // The button pins use the GPIO expanders internal pull up resistors so a
  // press is 0 and a 1 is released. Flip the bits so 1 is a press and 0 is a
  // release.
  new_bits.flip();
  return new_bits;
}

}  // namespace kudzu
