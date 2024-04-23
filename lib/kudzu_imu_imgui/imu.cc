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

#include "kudzu_imu/imu.h"

#include <math.h>

#include <cinttypes>

#define PW_LOG_MODULE_NAME "kudzu_imu_imgui"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "kudzu_imu_imgui/imu.h"
#include "pw_log/log.h"

namespace kudzu::imu {

PollingImuImGui::PollingImuImGui() {}

pw::Status PollingImuImGui::Init() { return pw::Status::Unimplemented(); }

bool PollingImuImGui::IsAvailable() { return true; }

pw::Result<ImuSample> PollingImuImGui::ReadData() {
  ImuSample data = {.accelerometer = {1.0f, 2.0f, 3.0f},
                    .gyroscope = {10, 20, 30}};

  last_data = data;
  return data;
}

}  // namespace kudzu::imu
