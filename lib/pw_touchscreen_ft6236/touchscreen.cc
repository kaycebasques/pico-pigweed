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

#include "pw_touchscreen/touchscreen.h"

#include <math.h>

#include <cinttypes>

#define PW_LOG_MODULE_NAME "pw_touchscreen_ft6236"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "ft6236/device.h"
#include "pw_geometry/vector3.h"
#include "pw_log/log.h"
#include "pw_touchscreen_ft6236/touchscreen.h"

namespace pw::touchscreen {

TouchscreenFT6236::TouchscreenFT6236(
    pw::ft6236::Device* touch_screen_controller)
    : touch_screen_controller_(touch_screen_controller) {}

Status TouchscreenFT6236::Init() {
  touch_screen_controller_->Enable();
  touch_screen_controller_->LogControllerInfo();
  return OkStatus();
}

bool TouchscreenFT6236::Available() { return true; }

bool TouchscreenFT6236::NewTouchEvent() {
  if (touch_screen_controller_->TouchCount() > 0) {
    return true;
  }
  return false;
}

TouchEvent TouchscreenFT6236::GetTouchPoint() {
  TouchEvent event = {
      .type = TouchEventType::None,
      .point = {0, 0, 0},
  };

  Result<bool> read_result = touch_screen_controller_->ReadData();
  if (!read_result.ok()) {
    return event;
  }

  if (touch_screen_controller_->TouchCount() > 0) {
    pw::ft6236::Touch touch_point1 = touch_screen_controller_->Touch1();
    event.point.x = touch_point1.x;
    event.point.y = touch_point1.y;
    event.point.z = 1;
  }

  if (last_touch_event.point.z == 0 && event.point.z == 1) {
    event.type = TouchEventType::Start;
    PW_LOG_DEBUG("Touch Start: x:%d, y:%d, z:%d",
                 event.point.x,
                 event.point.y,
                 event.point.z);
  } else if (last_touch_event.point.z == 1 && event.point.z == 1) {
    event.type = TouchEventType::Drag;
    PW_LOG_DEBUG("Touch Drag : x:%d, y:%d, z:%d",
                 event.point.x,
                 event.point.y,
                 event.point.z);
  } else if (last_touch_event.point.z == 1 && event.point.z == 0) {
    event.type = TouchEventType::Stop;
    PW_LOG_DEBUG("Touch Stop : x:%d, y:%d, z:%d",
                 event.point.x,
                 event.point.y,
                 event.point.z);
  }

  // TODO(tonymd): Touchscreen should accept parameters on how to transform
  // mouse to screen coordinates.
  if (event.type == TouchEventType::Start ||
      event.type == TouchEventType::Drag) {
    int new_x = round(((float)event.point.y / 320.0) * 160.0);
    int new_y = round(((240.0 - (float)event.point.x) / 240.0) * 120.0);
    event.point.x = new_x;
    event.point.y = new_y;
  }

  last_touch_event = event;
  return event;
}

}  // namespace pw::touchscreen
