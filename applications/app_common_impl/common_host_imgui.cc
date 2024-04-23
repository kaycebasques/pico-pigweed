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
#include "app_common/common.h"
#include "kudzu_buttons_imgui/buttons.h"
#include "kudzu_imu_imgui/imu.h"
#include "pw_color/color.h"
#include "pw_display_driver_imgui/display_driver.h"
#include "pw_display_imgui/display.h"
#include "pw_framebuffer_pool/framebuffer_pool.h"
#include "pw_status/status.h"
#include "pw_status/try.h"
#include "pw_thread/thread.h"
#include "pw_thread_stl/options.h"
#include "pw_touchscreen_imgui/touchscreen.h"

using pw::Status;
using pw::color::color_rgb565_t;
using pw::framebuffer::PixelFormat;
using pw::framebuffer_pool::FramebufferPool;

using Touchscreen = pw::touchscreen::TouchscreenImGui;
using Buttons = kudzu::ButtonsImgui;

namespace {

constexpr uint16_t kDisplayScaleFactor = 2;
constexpr pw::geometry::Size<uint16_t> kFramebufferDimensions = {
    .width = DISPLAY_WIDTH / kDisplayScaleFactor,
    .height = DISPLAY_HEIGHT / kDisplayScaleFactor,
};
constexpr size_t kNumPixels =
    kFramebufferDimensions.width * kFramebufferDimensions.height;
constexpr uint16_t kFramebufferRowBytes =
    sizeof(color_rgb565_t) * kFramebufferDimensions.width;
constexpr pw::geometry::Size<uint16_t> kDisplaySize = {DISPLAY_WIDTH,
                                                       DISPLAY_HEIGHT};

color_rgb565_t s_pixel_data[kNumPixels];
const pw::Vector<void*, 1> s_pixel_buffers{s_pixel_data};
FramebufferPool s_fb_pool({
    .fb_addr = s_pixel_buffers,
    .dimensions = kFramebufferDimensions,
    .row_bytes = kFramebufferRowBytes,
    .pixel_format = PixelFormat::RGB565,
});
pw::display_driver::DisplayDriverImgUI s_display_driver;

}  // namespace

// static
Status Common::EndOfFrameCallback() { return pw::OkStatus(); }

Status Common::Init() {
  auto status = s_display_driver.Init();
  if (!status.ok()) {
    return status;
  }
  status = GetButtons().Init();
  if (!status.ok()) {
    return status;
  }
  return pw::OkStatus();
}

// static
pw::display::Display& Common::GetDisplay() {
  static pw::display::DisplayImgUI s_display(
      s_display_driver, kDisplaySize, s_fb_pool);
  return s_display;
}

pw::touchscreen::Touchscreen& Common::GetTouchscreen() {
  static Touchscreen s_touchscreen = Touchscreen(s_display_driver);
  return s_touchscreen;
}

kudzu::Buttons& Common::GetButtons() {
  static Buttons s_buttons = Buttons(s_display_driver);
  return s_buttons;
}

kudzu::imu::PollingImu& Common::GetImu() {
  static kudzu::imu::PollingImuImGui s_imu = kudzu::imu::PollingImuImGui();
  return s_imu;
}

const pw::thread::Options& Common::DisplayDrawThreadOptions() {
  static pw::thread::stl::Options display_draw_thread_options;
  return display_draw_thread_options;
}

const pw::thread::Options& Common::TouchscreenThreadOptions() {
  static pw::thread::stl::Options display_draw_thread_options;
  return display_draw_thread_options;
}
