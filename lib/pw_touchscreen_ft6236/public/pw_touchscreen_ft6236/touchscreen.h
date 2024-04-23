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

#include "pw_geometry/vector3.h"
#include "pw_status/status.h"
#include "pw_touchscreen/touchscreen.h"

namespace pw::touchscreen {

class TouchscreenFT6236 : public Touchscreen {
 public:
  TouchscreenFT6236(pw::ft6236::Device* touch_screen_controller);

  Status Init() override;
  bool Available() override;
  bool NewTouchEvent() override;
  TouchEvent GetTouchPoint() override;

  TouchEvent last_touch_event;

 private:
  pw::ft6236::Device* touch_screen_controller_;
};

}  // namespace pw::touchscreen
