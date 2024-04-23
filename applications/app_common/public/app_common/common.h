// Copyright 2022 The Pigweed Authors
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
#include "kudzu_imu/imu.h"
#include "pw_display/display.h"
#include "pw_status/status.h"
#include "pw_thread/thread.h"
#include "pw_touchscreen/touchscreen.h"

// This class is used for initialization and to create the objects which
// are common to the test applications.
class Common {
 public:
  Common() = delete;

  // Initialize application common objects. This must be called before
  // any other methods in this class.
  static pw::Status Init();

  static pw::Status EndOfFrameCallback();

  // Return an initialized display.
  static pw::display::Display& GetDisplay();

  // Return an initialized display.
  static kudzu::imu::PollingImu& GetImu();

  // Return an initialized touchscreen.
  static pw::touchscreen::Touchscreen& GetTouchscreen();

  static kudzu::Buttons& GetButtons();

  // Provides thread options for the display thread.
  static const pw::thread::Options& DisplayDrawThreadOptions();

  // Provides thread options for the touchscreen thread.
  static const pw::thread::Options& TouchscreenThreadOptions();
};
