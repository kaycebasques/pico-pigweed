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
#include <chrono>
#include <cstdint>

#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "app_common/common.h"
#include "graphics/surface.hpp"
#include "libkudzu/framecounter.h"
#include "libkudzu/random.h"
#include "pw_assert/assert.h"
#include "pw_assert/check.h"
#include "pw_color/color.h"
#include "pw_color/colors_endesga32.h"
#include "pw_color/colors_pico8.h"
#include "pw_display/display.h"
#include "pw_framebuffer/framebuffer.h"
#include "pw_log/log.h"
#include "pw_string/string_builder.h"
#include "pw_sys_io/sys_io.h"
#include "pw_system/target_hooks.h"
#include "pw_thread/detached_thread.h"

using pw::color::color_rgb565_t;
using pw::color::kColorsPico8Rgb565;
using pw::display::Display;
using pw::framebuffer::Framebuffer;

namespace {

struct test_particle {
  blit::Vec2 pos;
  blit::Vec2 vel;
  int age;
  bool generated = false;
};

void rain_generate(test_particle& p, blit::Surface screen) {
  p.pos = blit::Vec2(GetRandomFloat(screen.bounds.w),
                     GetRandomFloat(10) - (screen.bounds.h + 10));
  p.vel = blit::Vec2(0, 150);
  p.age = 0;
  p.generated = true;
};

void rain(blit::Surface screen,
          pw::chrono::SystemClock::duration elapsed_time,
          blit::Rect floor_position) {
  static test_particle s[300];
  static int generate_index = 0;

  // Convert to fractional elapsed seconds.
  auto const elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<float>>(elapsed_time);

  rain_generate(s[generate_index++], screen);
  if (generate_index >= 300)
    generate_index = 0;

  blit::Vec2 gvec = blit::Vec2(0, 9.8 * 5);
  blit::Vec2 gravity = gvec * elapsed_seconds.count();

  for (auto& p : s) {
    if (p.generated) {
      p.vel += gravity;
      p.pos += p.vel * elapsed_seconds.count();

      int floor = -3;
      if (p.pos.x > floor_position.x &&
          p.pos.x < (floor_position.x + floor_position.w))
        floor = -3 - (screen.bounds.h - floor_position.y);

      if (p.pos.y >= floor) {
        p.pos.y = floor;
        float bounce = (GetRandomFloat(10)) / 80.0f;
        p.vel.y *= -bounce;
        p.vel.x = (GetRandomFloat(30) - 15);
      }
      p.age++;

      int a = p.age / 2;
      int r = 100 - (a / 2);
      int g = 255 - (a / 2);
      int b = 255;  // -(a * 4);

      if (p.vel.length() > 20) {
        screen.pen = blit::Pen(b, g, r, 100);
        screen.pixel(p.pos + blit::Point(0, screen.bounds.h - 1));
        screen.pen = blit::Pen(b, g, r, 160);
        screen.pixel(p.pos + blit::Point(0, screen.bounds.h + 1));
      }
      screen.pen = blit::Pen(b, g, r, 180);
      screen.pixel(p.pos + blit::Point(0, screen.bounds.h + 2));
    }
  }
};

void MainTask(void*) {
  // Timing variables
  kudzu::FrameCounter frame_counter = kudzu::FrameCounter();

  PW_CHECK_OK(Common::Init());

  Display& display = Common::GetDisplay();
  Framebuffer framebuffer = display.GetFramebuffer();
  PW_ASSERT(framebuffer.is_valid());

  blit::Surface screen = blit::Surface(
      (uint8_t*)framebuffer.data(),
      blit::PixelFormat::RGB565,
      blit::Size(framebuffer.size().width, framebuffer.size().height));
  screen.pen = blit::Pen(0, 0, 0, 255);
  screen.clear();

  display.ReleaseFramebuffer(std::move(framebuffer));

  // The display loop.
  while (1) {
    frame_counter.StartFrame();

    framebuffer = display.GetFramebuffer();
    PW_ASSERT(framebuffer.is_valid());
    screen.data = (uint8_t*)framebuffer.data();

    // Draw Phase
    // Clear the screen
    screen.pen = blit::Pen(0, 0, 0);
    screen.clear();

    // Draw 32blit animation
    std::string text = "Pigweed + 32blit";
    auto text_size = screen.measure_text(text, blit::minimal_font, true);
    blit::Rect text_rect(
        blit::Point((screen.bounds.w / 2) - (text_size.w / 2),
                    (screen.bounds.h * .75) - (text_size.h / 2)),
        text_size);

    rain(screen, frame_counter.LastFrameDuration(), text_rect);
    screen.pen = blit::Pen(0xFF, 0xFF, 0xFF);
    screen.text(
        text, blit::minimal_font, text_rect, true, blit::TextAlign::top_left);

    // Update timers
    frame_counter.EndDraw();

    display.ReleaseFramebuffer(std::move(framebuffer));
    frame_counter.EndFlush();

    // Every second make a log message.
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
