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
#include <cstdint>

#define PW_LOG_MODULE_NAME "SnakeGame"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "app_common/common.h"
#include "graphics/surface.hpp"
#include "libkudzu/framecounter.h"
#include "pw_assert/check.h"
#include "pw_display/display.h"
#include "pw_framebuffer/framebuffer.h"
#include "pw_log/log.h"
#include "pw_system/target_hooks.h"
#include "pw_system/work_queue.h"
#include "pw_thread/detached_thread.h"
#include "pw_thread/thread_core.h"
#include "pw_touchscreen/buttons.h"
#include "pw_touchscreen/touchscreen.h"
#include "snake/game.h"

namespace {

class PollingTouchButtonsThread : public pw::thread::ThreadCore {
 public:
  PollingTouchButtonsThread(
      pw::touchscreen::Touchscreen& touchscreen,
      pw::touchscreen::DirectionButtonListener& button_listener,
      int32_t display_width,
      int32_t display_height)
      : touchscreen_(touchscreen),
        buttons_(button_listener, display_width, display_height) {}

  void Run() override {
    while (true) {
      pw::touchscreen::TouchEvent touch_event = touchscreen_.GetTouchPoint();
      buttons_.OnTouchEvent(touch_event);
    }
  }

 private:
  pw::touchscreen::Touchscreen& touchscreen_;
  pw::touchscreen::DirectionTouchButtons buttons_;
};

void MainTask(void*) {
  PW_CHECK_OK(Common::Init());

  pw::display::Display& display = Common::GetDisplay();
  pw::framebuffer::Framebuffer framebuffer = display.GetFramebuffer();
  PW_CHECK(framebuffer.is_valid());

  blit::Surface screen = blit::Surface(
      static_cast<uint8_t*>(framebuffer.data()),
      blit::PixelFormat::RGB565,
      blit::Size(framebuffer.size().width, framebuffer.size().height));
  screen.pen = blit::Pen(0, 0, 0, 255);
  screen.clear();

  const auto display_width = framebuffer.size().width;
  const auto display_height = framebuffer.size().height;
  display.ReleaseFramebuffer(std::move(framebuffer));

  snake::Game game(display_width, display_height);
  PollingTouchButtonsThread touch_buttons_thread{
      Common::GetTouchscreen(), game, display_width, display_height};
  pw::thread::DetachedThread(Common::TouchscreenThreadOptions(),
                             touch_buttons_thread);

  game.Start();

  // Display and app loop.
  kudzu::FrameCounter frame_counter = kudzu::FrameCounter();
  while (true) {
    frame_counter.StartFrame();

    // Get frame buffer.
    framebuffer = display.GetFramebuffer();
    PW_CHECK(framebuffer.is_valid());
    screen.data = static_cast<uint8_t*>(framebuffer.data());
    // Clear the screen
    screen.pen = blit::Pen(0, 0, 0);
    screen.clear();

    // Let game draw its frame.
    game.OnFrame(framebuffer);

    // Update timers
    frame_counter.EndDraw();

    display.ReleaseFramebuffer(std::move(framebuffer));
    frame_counter.EndFlush();

    frame_counter.LogTiming();
  }
}

}  // namespace

namespace pw::system {

void UserAppInit() {
  PW_LOG_INFO("UserAppInit");
  pw::thread::DetachedThread(Common::DisplayDrawThreadOptions(), MainTask);
}

}  // namespace pw::system
