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

#include "pw_result/result.h"
#include "pw_status/status.h"

namespace kudzu::imu {

struct AccelerometerData {
  float x, y, z;  // g / axis
};

struct GyroscopeData {
  int16_t x, y, z;  // dps / axis
};

struct ImuSample {
  AccelerometerData accelerometer;
  GyroscopeData gyroscope;
};

// Abstract base class for Polling IMU
class PollingImu {
 public:
  virtual ~PollingImu() = default;

  // Initialize the IMU controller.
  virtual pw::Status Init() = 0;

  // Return true if the controller is ready.
  virtual bool IsAvailable() = 0;

  // Retrieve IMU data. The concrete implementation would typically communicate
  // with the actual IMU hardware to get this data.
  virtual pw::Result<ImuSample> ReadData() = 0;
};

}  // namespace kudzu::imu
