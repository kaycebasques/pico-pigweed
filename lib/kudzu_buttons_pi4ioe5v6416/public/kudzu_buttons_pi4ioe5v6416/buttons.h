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

#include "kudzu_buttons/buttons.h"
#include "pi4ioe5v6416/device.h"
#include "pw_status/status.h"

namespace kudzu {

class ButtonsPI4IOE5V6416 : public Buttons {
 public:
  ButtonsPI4IOE5V6416(pw::pi4ioe5v6416::Device* controller);
  pw::Status Init() override;
  pw::Result<std::bitset<kButtonCount>> DoUpdate() override;

 private:
  pw::pi4ioe5v6416::Device* controller_;
};

}  // namespace kudzu
