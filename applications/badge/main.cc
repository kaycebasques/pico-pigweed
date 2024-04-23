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

#include "pw_banner46x10.h"

#define PW_LOG_MODULE_NAME "Badge"
#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "app_common/common.h"
#include "graphics/surface.hpp"
#include "heart_8x8.h"
#include "hello_my_name_is65x42.h"
#include "kudzu_buttons/buttons.h"
#include "kudzu_isometric_text_sprite.h"
#include "libkudzu/framecounter.h"
#include "libkudzu/random.h"
#include "name_tag.h"
#include "pw_assert/assert.h"
#include "pw_assert/check.h"
#include "pw_banner46x10.h"
#include "pw_color/color.h"
#include "pw_color/colors_endesga32.h"
#include "pw_color/colors_pico8.h"
#include "pw_display/display.h"
#include "pw_draw/draw.h"
#include "pw_draw/font6x8.h"
#include "pw_framebuffer/framebuffer.h"
#include "pw_geometry/vector2.h"
#include "pw_log/log.h"
#include "pw_logo5x7.h"
#include "pw_string/string_builder.h"
#include "pw_sys_io/sys_io.h"
#include "pw_system/target_hooks.h"
#include "pw_thread/detached_thread.h"
#include "pw_touchscreen/touchscreen.h"

using kudzu::Buttons;
using pw::color::color_rgb565_t;
using pw::color::kColorsPico8Rgb565;
using pw::display::Display;
using pw::framebuffer::Framebuffer;
using pw::geometry::Size;
using pw::geometry::Vector2;
using pw::touchscreen::Touchscreen;

namespace {

constexpr float pi = 3.14159265358979323846f;
constexpr float twopi = 2 * pi;
constexpr float angle_step = twopi / 120;
float angle = 0;

bool show_nametag = false;
bool show_background = false;

// Draw the a waving text banner.
// Returns the bottom Y coordinate of the bottommost pixel set.
void DrawTextBanner(Framebuffer& framebuffer) {
  constexpr std::array<std::wstring_view, 2> banner = {
      L"Hello World from KUDZU!",
  };
  auto font = pw::draw::GetFont6x8();

  Vector2<int> tl = {12, 32};

  int color_index = 0;
  const float y_scale = 4.0;
  const float x_scale = 1.0;
  const float max_x_offset = 4.0;
  const float max_y_offset = font.height * 2.0;

  for (auto text_row : banner) {
    int column = 0;
    for (auto text_char : text_row) {
      Vector2<int> position = {tl.x, tl.y};

      float final_angle = angle + (y_scale * column * angle_step);
      if (final_angle > twopi)
        final_angle -= twopi;
      if (final_angle < 0)
        final_angle += twopi;

      float offset_y = std::sin(final_angle);
      float offset_x = std::cos(angle + (x_scale * column * angle_step));

      position.x += round(max_x_offset * offset_x);
      position.y += round(max_y_offset * offset_y);

      pw::draw::DrawCharacter(text_char,
                              position,
                              kColorsPico8Rgb565[color_index + 8],
                              0,
                              font,
                              framebuffer);

      if (text_char != ' ') {
        // Loop through these pico8 colors:
        // 0xf809,  // #ff004d 8 RED
        // 0xfd00,  // #ffa300 9 ORANGE
        // 0xff64,  // #ffec27 10 YELLOW
        // 0x0726,  // #00e436 11 GREEN
        // 0x2d7f,  // #29adff 12 BLUE
        // 0x83b3,  // #83769c 13 INDIGO
        // 0xfbb5,  // #ff77a8 14 PINK
        // 0xfe75,  // #ffccaa 15 PEACH
        color_index = (color_index + 1) % 7;
      }

      tl.x += font.width;
      column += 1;
    }
    tl.x = 0;
    tl.y += font.height;
  }
}

void DrawKudzu(Framebuffer& framebuffer,
               float y_scale_offset = 0.0,
               float x_scale_offset = 0.0) {
  Vector2<int> tl = {0, 16};

  const float y_scale = 12.0;
  const float x_scale = 1.0;
  const float max_x_offset = 4.0 + x_scale_offset;
  const float max_y_offset = 16.0 + y_scale_offset;

  // X offsets between each letter
  std::array<int, 5> x_offsets = {32, 22, 26, 30, 0};
  // Original y offsets:
  // std::array<int, 5> y_offsets = {14, 13, 13, 13, 0};
  // y offsets / 2
  std::array<int, 5> y_offsets = {7, 6, 6, 6, 0};

  for (int column = 0; column < 5; column++) {
    Vector2<int> position = {tl.x, tl.y};

    float final_angle = angle + (y_scale * column * angle_step);
    if (final_angle > twopi)
      final_angle -= twopi;
    if (final_angle < 0)
      final_angle += twopi;

    float offset_y = std::sin(final_angle);
    float offset_x = std::cos(angle + (x_scale * column * angle_step));

    position.x += round(max_x_offset * offset_x);
    position.y += round(max_y_offset * offset_y);

    pw::draw::DrawSprite(framebuffer,
                         position.x,
                         position.y,
                         &kudzu_isometric_text_sprite_sprite_sheet,
                         /*integer_scale*/ 1);
    kudzu_isometric_text_sprite_sprite_sheet.RotateIndexLoop();
    tl.x += x_offsets[column];
    tl.y += y_offsets[column];
  }
}

void DrawGreeting(Framebuffer& framebuffer, blit::Surface& screen) {
  std::string text = "Nice to meet you\n Made with * by";
  auto text_size = screen.measure_text(text, blit::minimal_font, true);
  blit::Rect text_rect(blit::Point((screen.bounds.w / 2) - (text_size.w / 2),
                                   (screen.bounds.h * .8) - (text_size.h / 2)),
                       text_size);
  screen.pen = blit::Pen(0xFF, 0xFF, 0xFF);
  screen.text(
      text, blit::minimal_font, text_rect, true, blit::TextAlign::top_left);

  pw::draw::DrawSprite(framebuffer,
                       text_rect.x + text_rect.w - 29,
                       text_rect.y + text_rect.h - 9,
                       &heart_8x8_sprite_sheet,
                       1);
  pw::draw::DrawSprite(framebuffer,
                       text_rect.x + 10,
                       text_rect.y + text_rect.h + 2,
                       &pw_logo5x7_sprite_sheet,
                       1);
  pw::draw::DrawSprite(framebuffer,
                       text_rect.x + 10 + 7,
                       text_rect.y + text_rect.h,
                       &pw_banner46x10_sprite_sheet,
                       1);
}

void DrawNametag(Framebuffer& framebuffer, blit::Surface& screen) {
  blit::Point tag_position(0, 0);
  blit::Rect outer_tag_rect(tag_position, screen.bounds);

  screen.pen = blit::Pen(0x4d, 0x00, 0xff);
  screen.rectangle(outer_tag_rect);

  pw::draw::DrawSprite(
      framebuffer, 47, 6, &hello_my_name_is65x42_sprite_sheet, 1);

  int name_rect_y_offset = hello_my_name_is65x42_sprite_sheet.height + 6 + 4;
  tag_position += blit::Point(4, name_rect_y_offset);
  blit::Size name_size(screen.bounds.w - 8,
                       screen.bounds.h - name_rect_y_offset - 4);

  blit::Rect name_rect(tag_position, name_size);
  screen.pen = blit::Pen(0xff, 0xff, 0xff);
  screen.rectangle(name_rect);

  pw::draw::DrawSprite(
      framebuffer, tag_position.x, tag_position.y, &name_tag_sprite_sheet, 1);
}

void DrawBackgroundColors(Framebuffer& framebuffer) {
  static color_rgb565_t base_color = 0;
  static uint16_t magic = 27;

  uint16_t* p = static_cast<uint16_t*>(framebuffer.data());
  for (int y = 0; y < framebuffer.size().height; y++) {
    for (int x = 0; x < framebuffer.size().width; x++) {
      *p++ = base_color + magic * (x ^ y);
    }
  }
  base_color += 0x0021;
}

void MainTask(void*) {
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

  Touchscreen& touchscreen = Common::GetTouchscreen();
  pw::touchscreen::TouchEvent last_touch_event;

  Buttons& kudzu_buttons = Common::GetButtons();

  float x_scale_offset = 0.0;
  float y_scale_offset = 0.0;
  const float x_scale_increment = 0.7;
  const float y_scale_increment = 0.7;
  // The display loop.
  while (1) {
    frame_counter.StartFrame();

    angle += angle_step;
    if (angle >= twopi) {
      angle = 0;
    }

    pw::touchscreen::TouchEvent touch_event = touchscreen.GetTouchPoint();

    framebuffer = display.GetFramebuffer();
    PW_ASSERT(framebuffer.is_valid());
    screen.data = (uint8_t*)framebuffer.data();

    // Draw Phase
    // Clear the screen
    screen.pen = blit::Pen(0, 0, 0);
    screen.clear();

    // Mode switch button
    blit::Size button_size(48, 36);
    blit::Point button_position(screen.bounds.w - button_size.w, 0);
    blit::Rect mode_button_rect(button_position, button_size);

    kudzu_buttons.Update();
    if (kudzu_buttons.Pressed(kudzu::button::a)) {
      show_nametag = !show_nametag;
    }
    if (kudzu_buttons.Pressed(kudzu::button::b)) {
      show_background = !show_background;
    }
    if (kudzu_buttons.Pressed(kudzu::button::start)) {
      x_scale_offset = 0;
      y_scale_offset = 0;
    }
    if (kudzu_buttons.Held(kudzu::button::left)) {
      y_scale_offset += y_scale_increment;
    } else if (kudzu_buttons.Held(kudzu::button::right)) {
      y_scale_offset -= y_scale_increment;
    }
    if (kudzu_buttons.Held(kudzu::button::up)) {
      x_scale_offset += x_scale_increment;
    } else if (kudzu_buttons.Held(kudzu::button::down)) {
      x_scale_offset -= x_scale_increment;
    }

    if (show_nametag) {
      DrawNametag(framebuffer, screen);
      // Draw button
      screen.pen = blit::Pen(255, 255, 255);
      screen.text("kudzu!",
                  blit::minimal_font,
                  mode_button_rect,
                  true,
                  blit::TextAlign::top_right);
    } else {
      if (show_background) {
        DrawBackgroundColors(framebuffer);
      }
      DrawKudzu(framebuffer, x_scale_offset, y_scale_offset);
      DrawGreeting(framebuffer, screen);

      // Draw button
      screen.pen = blit::Pen(255, 0, 255);
      screen.text("hello!",
                  blit::minimal_font,
                  mode_button_rect,
                  true,
                  blit::TextAlign::top_right);
    }

    if (touch_event.type == pw::touchscreen::TouchEventType::Start ||
        touch_event.type == pw::touchscreen::TouchEventType::Drag) {
      pw::draw::DrawCircle(framebuffer,
                           touch_event.point.x,
                           touch_event.point.y,
                           18,
                           kColorsPico8Rgb565[pw::color::kColorBlue],
                           false);
    }
    if (last_touch_event.type == pw::touchscreen::TouchEventType::Drag &&
        touch_event.type == pw::touchscreen::TouchEventType::Stop) {
      PW_LOG_DEBUG("Touch Stop at: %d, %d",
                   last_touch_event.point.x,
                   last_touch_event.point.y);
      if (mode_button_rect.contains(blit::Point(last_touch_event.point.x,
                                                last_touch_event.point.y))) {
        show_nametag = !show_nametag;
      }
    }

    last_touch_event = touch_event;

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
