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

#include "kudzu_imu/imu.h"
#include "pw_status/status.h"

namespace kudzu::imu {

class PollingImuICM42670P : public PollingImu {
 public:
  PollingImuICM42670P(kudzu::icm42670p::Device* imu_controller);

  pw::Status Init() override;
  bool IsAvailable() override;
  pw::Result<ImuSample> ReadData() override;

  ImuSample last_data;

 private:
  kudzu::icm42670p::Device* imu_controller_;
};

}  // namespace kudzu::imu