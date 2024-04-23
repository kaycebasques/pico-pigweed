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

#include <cstdint>

#include "pw_touchscreen/touchscreen.h"
#include "types/point.hpp"
#include "types/rect.hpp"
#include "types/size.hpp"

namespace pw::touchscreen {

// Represents a button on the touch screen.
class TouchButton {
 public:
  TouchButton(int32_t width, int32_t height, int32_t x, int32_t y)
      : size_(width, height), position_(x, y), rect_(position_, size_) {}

  bool contains(int32_t x, int32_t y) {
    return rect_.contains(blit::Point(x, y));
  }

 private:
  blit::Size size_;
  blit::Point position_;
  blit::Rect rect_;
};

// Listens to Direction Button events, i.e. up, down, left, and right.
class DirectionButtonListener {
 public:
  virtual ~DirectionButtonListener() = default;

  // Performs an action on button a button press (``pressed`` is true) or
  // released (``pressed`` is false).
  virtual void OnButtonUp(bool pressed) = 0;
  virtual void OnButtonDown(bool pressed) = 0;
  virtual void OnButtonLeft(bool pressed) = 0;
  virtual void OnButtonRight(bool pressed) = 0;
};

// Creates a set of direction buttons (up, down, left, and right) on the given
// touch screen and redirects touch events to button listener when applicable.
class DirectionTouchButtons {
 public:
  DirectionTouchButtons(DirectionButtonListener& button_listener,
                        int32_t display_width,
                        int32_t display_height);

  void OnTouchEvent(const TouchEvent& touch_event);

 private:
  DirectionButtonListener& button_listener_;
  TouchButton up_button_;
  TouchButton down_button_;
  TouchButton left_button_;
  TouchButton right_button_;
};

}  // namespace pw::touchscreen
