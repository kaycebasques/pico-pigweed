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

#define PW_LOG_MODULE_NAME "kudzu_imu_icm42670p"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "icm42670p/device.h"
#include "kudzu_imu_icm42670p/imu.h"
#include "pw_log/log.h"

namespace kudzu::imu {

PollingImuICM42670P::PollingImuICM42670P(
    kudzu::icm42670p::Device* imu_controller)
    : imu_controller_(imu_controller) {}

pw::Status PollingImuICM42670P::Init() {
  imu_controller_->Enable();
  imu_controller_->LogControllerInfo();
  return pw::OkStatus();
}

bool PollingImuICM42670P::IsAvailable() { return true; }

pw::Result<ImuSample> PollingImuICM42670P::ReadData() {
  pw::Result<ImuSample> data = imu_controller_->ReadValues();

  if (data.ok()) {
    last_data = data.value();
  }
  return data;
}

}  // namespace kudzu::imu
