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

namespace pw::touchscreen {

enum TouchEventType {
  None,
  Start,
  Stop,
  Drag,
};

struct TouchEvent {
  TouchEventType type = TouchEventType::None;
  pw::geometry::Vector3<int> point = {0, 0, 0};
};

class Touchscreen {
 public:
  virtual ~Touchscreen() = default;

  // Initialize the touchscreen controller.
  virtual Status Init() = 0;

  // Return true if the controller is ready.
  virtual bool Available() = 0;

  // Return true if there is a new touch event.
  virtual bool NewTouchEvent() = 0;

  // Return x, y, and z coordinate of the current touch event.
  virtual TouchEvent GetTouchPoint() = 0;
};

}  // namespace pw::touchscreen
