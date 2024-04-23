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
#include "pw_touchscreen/buttons.h"

#include <cstdint>

#include "pw_log/log.h"
#include "pw_touchscreen/touchscreen.h"

namespace pw::touchscreen {
namespace {

const char* kPressedText = "pressed";
const char* kReleasedText = "released";

}  // namespace

DirectionTouchButtons::DirectionTouchButtons(
    DirectionButtonListener& button_listener,
    int32_t display_width,
    int32_t display_height)
    : button_listener_(button_listener),
      up_button_(display_width / 2, display_height / 4, display_width / 4, 0),
      down_button_(display_width / 2,
                   display_height / 4,
                   display_width / 4,
                   display_height - display_height / 4),
      left_button_(
          display_width / 4, display_height / 2, 0, display_height / 4),
      right_button_(display_width / 4,
                    display_height / 2,
                    display_width - display_width / 4,
                    display_height / 4) {}

void DirectionTouchButtons::OnTouchEvent(const TouchEvent& touch_event) {
  if (touch_event.type == TouchEventType::Start ||
      touch_event.type == TouchEventType::Stop) {
    const bool pressed = touch_event.type == TouchEventType::Start;
    PW_LOG_DEBUG("Touch at: %d, %d", touch_event.point.x, touch_event.point.y);
    if (up_button_.contains(touch_event.point.x, touch_event.point.y)) {
      PW_LOG_DEBUG("Up button %s", pressed ? kPressedText : kReleasedText);
      button_listener_.OnButtonUp(pressed);
    } else if (down_button_.contains(touch_event.point.x,
                                     touch_event.point.y)) {
      PW_LOG_DEBUG("Down button %s", pressed ? kPressedText : kReleasedText);
      button_listener_.OnButtonDown(pressed);
    } else if (left_button_.contains(touch_event.point.x,
                                     touch_event.point.y)) {
      PW_LOG_DEBUG("Left button %s", pressed ? kPressedText : kReleasedText);
      button_listener_.OnButtonLeft(pressed);
    } else if (right_button_.contains(touch_event.point.x,
                                      touch_event.point.y)) {
      PW_LOG_DEBUG("Right button %s", pressed ? kPressedText : kReleasedText);
      button_listener_.OnButtonRight(pressed);
    }
  }
}

}  // namespace pw::touchscreen
