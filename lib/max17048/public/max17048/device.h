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

#include <array>
#include <cstdint>

#include "pw_i2c/address.h"
#include "pw_i2c/initiator.h"
#include "pw_i2c/register_device.h"
#include "pw_status/status.h"

namespace pw::max17048 {

class Device {
 public:
  Device(pw::i2c::Initiator& initiator);
  ~Device() = default;

  Status Enable();
  Status Probe();
  void LogControllerInfo();

 private:
  pw::i2c::Initiator& initiator_;
  pw::i2c::RegisterDevice device_;
};

}  // namespace pw::max17048
