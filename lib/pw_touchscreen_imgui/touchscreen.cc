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

#include "pw_touchscreen_imgui/touchscreen.h"

#include <cinttypes>

#define PW_LOG_MODULE_NAME "pw_touchscreen_imgui"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "pw_display_driver_imgui/display_driver.h"
#include "pw_geometry/vector3.h"
#include "pw_log/log.h"
#include "pw_status/status.h"

namespace pw::touchscreen {

TouchscreenImGui::TouchscreenImGui(
    pw::display_driver::DisplayDriverImgUI& display_driver)
    : display_driver_(display_driver) {}

Status TouchscreenImGui::Init() { return OkStatus(); }

bool TouchscreenImGui::Available() { return true; }

bool TouchscreenImGui::NewTouchEvent() {
  auto mouse = display_driver_.GetImGuiMousePosition();
  return mouse.left_button_pressed;
}

TouchEvent TouchscreenImGui::GetTouchPoint() {
  TouchEvent event = {
      .type = TouchEventType::None,
      .point = {0, 0, 0},
  };

  auto mouse = display_driver_.GetImGuiMousePosition();
  if (mouse.left_button_pressed) {
    event.point.x = mouse.position_x;
    event.point.y = mouse.position_y;
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
    PW_LOG_DEBUG("Touch Stop");
  }

  last_touch_event = event;
  return event;
}

}  // namespace pw::touchscreen
