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

#include "pw_touchscreen_null/touchscreen.h"

#include <cinttypes>

#include "pw_geometry/vector3.h"
#include "pw_status/status.h"

namespace pw::touchscreen {

TouchscreenNull::TouchscreenNull() {}

Status TouchscreenNull::Init() { return OkStatus(); }

bool TouchscreenNull::Available() { return false; }

bool TouchscreenNull::NewTouchEvent() { return false; }

TouchEvent TouchscreenNull::GetTouchPoint() { return TouchEvent(); }

}  // namespace pw::touchscreen
