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
#include <array>
#include <cstdint>
#include <cwchar>
#include <forward_list>
#include <memory>
#include <string_view>
#include <utility>

#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG

#include "ansi.h"
#include "app_common/common.h"
#include "libkudzu/framecounter.h"
#include "pw_assert/assert.h"
#include "pw_assert/check.h"
#include "pw_color/color.h"
#include "pw_color/colors_endesga32.h"
#include "pw_color/colors_pico8.h"
#include "pw_draw/draw.h"
#include "pw_draw/font6x8.h"
#include "pw_draw/font_set.h"
#include "pw_draw/pigweed_farm.h"
#include "pw_framebuffer/framebuffer.h"
#include "pw_geometry/vector2.h"
#include "pw_geometry/vector3.h"
#include "pw_log/log.h"
#include "pw_ring_buffer/prefixed_entry_ring_buffer.h"
#include "pw_string/string_builder.h"
#include "pw_sys_io/sys_io.h"
#include "pw_system/target_hooks.h"
#include "pw_thread/detached_thread.h"
#include "pw_touchscreen/touchscreen.h"
#include "text_buffer.h"

using pw::color::color_rgb565_t;
using pw::color::kColorsPico8Rgb565;
using pw::display::Display;
using pw::draw::FontSet;
using pw::framebuffer::Framebuffer;
using pw::geometry::Size;
using pw::geometry::Vector2;

namespace {

constexpr color_rgb565_t kBlack = 0U;
constexpr color_rgb565_t kWhite = 0xffff;

class DemoDecoder : public AnsiDecoder {
 public:
  DemoDecoder(TextBuffer& log_text_buffer)
      : log_text_buffer_(log_text_buffer) {}

 protected:
  void SetFgColor(uint8_t r, uint8_t g, uint8_t b) override {
    fg_color_ = pw::color::ColorRgba(r, g, b).ToRgb565();
  }
  void SetBgColor(uint8_t r, uint8_t g, uint8_t b) override {
    bg_color_ = pw::color::ColorRgba(r, g, b).ToRgb565();
  }
  void EmitChar(char c) override {
    log_text_buffer_.DrawCharacter(TextBuffer::Char{c, fg_color_, bg_color_});
  }

 private:
  color_rgb565_t fg_color_ = kWhite;
  color_rgb565_t bg_color_ = kBlack;
  TextBuffer& log_text_buffer_;
};

// A simple implementation of a UI button.
class Button {
 public:
  // The label ptr must be valid throughout the lifetime of this object.
  Button(const wchar_t* label, const Vector2<int>& tl, const Size<int>& size)
      : label_(label), tl_(tl), size_(size) {}

  bool Contains(Vector2<int> pt) const {
    return pt.x >= tl_.x && pt.x < (tl_.x + size_.width) && pt.y >= tl_.y &&
           pt.y < (tl_.y + size_.height);
  }
  const wchar_t* label_;
  const Vector2<int> tl_;
  const Size<int> size_;
};

constexpr const wchar_t* kButtonLabel = L"Click to add logs";
constexpr int kButtonWidth = 108;
constexpr Vector2<int> kButtonTL = {320 - kButtonWidth, 0};
constexpr Size<int> kButtonSize = {kButtonWidth, 12};

TextBuffer s_log_text_buffer;
DemoDecoder s_demo_decoder(s_log_text_buffer);
Button g_button(kButtonLabel, kButtonTL, kButtonSize);

void DrawButton(const Button& button,
                color_rgb565_t bg_color,
                Framebuffer& framebuffer) {
  pw::draw::DrawRectWH(framebuffer,
                       button.tl_.x,
                       button.tl_.y,
                       button.size_.width,
                       button.size_.height,
                       bg_color,
                       /*filled=*/true);
  constexpr int kMargin = 2;
  Vector2<int> tl{button.tl_.x + kMargin, button.tl_.y + kMargin};
  pw::draw::DrawString(
      button.label_, tl, kBlack, bg_color, pw::draw::GetFont6x8(), framebuffer);
}

// Draw a font sheet starting at the given top-left screen coordinates.
Vector2<int> DrawTestFontSheet(Vector2<int> tl,
                               int num_columns,
                               color_rgb565_t fg_color,
                               color_rgb565_t bg_color,
                               const FontSet& font,
                               Framebuffer& framebuffer) {
  Vector2<int> max_extents = tl;
  const int initial_x = tl.x;
  for (int c = font.starting_character; c <= font.ending_character; c++) {
    int char_idx = c - font.starting_character;
    if (char_idx % num_columns == 0) {
      tl.x = initial_x;
      tl.y += font.height;
    }
    auto char_size =
        DrawCharacter(c, tl, fg_color, bg_color, font, framebuffer);
    tl.x += char_size.width;
    max_extents.x = std::max(tl.x, max_extents.x);
    max_extents.y = std::max(tl.y, max_extents.y);
  }
  max_extents.y += font.height;
  return max_extents;
}

Vector2<int> DrawColorFontSheet(Vector2<int> tl,
                                int num_columns,
                                color_rgb565_t fg_color,
                                const FontSet& font,
                                Framebuffer& framebuffer) {
  constexpr int kNumColors = sizeof(pw::color::colors_endesga32_rgb565) /
                             sizeof(pw::color::colors_endesga32_rgb565[0]);
  const int initial_x = tl.x;
  Vector2<int> max_extents = tl;
  for (int c = font.starting_character; c <= font.ending_character; c++) {
    int char_idx = c - font.starting_character;
    if (char_idx % num_columns == 0) {
      tl.x = initial_x;
      tl.y += font.height;
    }
    auto char_size =
        DrawCharacter(c,
                      tl,
                      fg_color,
                      pw::color::colors_endesga32_rgb565[char_idx % kNumColors],
                      font,
                      framebuffer);
    tl.x += char_size.width;
    max_extents.x = std::max(tl.x, max_extents.x);
    max_extents.y = std::max(tl.y, max_extents.y);
  }
  max_extents.y += font.height;
  return max_extents;
}

// The logging callback used to capture log messages sent to pw_log.
void LogCallback(std::string_view log) {
  for (auto c : log) {
    s_demo_decoder.ProcessChar(c);
  }
  s_demo_decoder.ProcessChar('\n');

  pw::sys_io::WriteLine(log).IgnoreError();
};

// Draw the Pigweed sprite and artwork at the top of the display.
// Returns the bottom Y coordinate drawn.
int DrawPigweedSprite(Framebuffer& framebuffer) {
  int sprite_pos_x = 10;
  int sprite_pos_y = 24;
  int sprite_scale = 4;
  // int border_size = 8;

  // // Draw the dark blue border
  // pw::draw::DrawRectWH(
  //     framebuffer,
  //     sprite_pos_x - border_size,
  //     sprite_pos_y - border_size,
  //     pigweed_farm_sprite_sheet.width * sprite_scale + (border_size * 2),
  //     pigweed_farm_sprite_sheet.height * sprite_scale + (border_size * 2),
  //     kColorsPico8Rgb565[pw::color::kColorDarkBlue],
  //     true);

  // // Shrink the border
  // border_size = 4;

  // // Draw the light blue background
  // pw::draw::DrawRectWH(
  //     framebuffer,
  //     sprite_pos_x - border_size,
  //     sprite_pos_y - border_size,
  //     pigweed_farm_sprite_sheet.width * sprite_scale + (border_size * 2),
  //     pigweed_farm_sprite_sheet.height * sprite_scale + (border_size * 2),
  //     kColorsPico8Rgb565[pw::color::kColorBlue],
  //     true);

  static Vector2<int> sun_offset;
  static int motion_dir = -1;
  static int frame_num = 0;
  frame_num++;
  if ((frame_num % 5) == 0)
    sun_offset.x += motion_dir;
  if ((frame_num % 15) == 0)
    sun_offset.y -= motion_dir;
  if (sun_offset.x < -100)
    motion_dir = 1;
  else if (sun_offset.x > 10)
    motion_dir = -1;

  // Draw the Sun
  pw::draw::DrawCircle(framebuffer,
                       sun_offset.x + sprite_pos_x +
                           (pigweed_farm_sprite_sheet.width * sprite_scale) -
                           32,
                       sun_offset.y + sprite_pos_y,
                       20,
                       kColorsPico8Rgb565[pw::color::kColorOrange],
                       true);
  pw::draw::DrawCircle(framebuffer,
                       sun_offset.x + sprite_pos_x +
                           (pigweed_farm_sprite_sheet.width * sprite_scale) -
                           32,
                       sun_offset.y + sprite_pos_y,
                       18,
                       kColorsPico8Rgb565[pw::color::kColorYellow],
                       true);

  // // Draw the farm sprite's shadow
  // pigweed_farm_sprite_sheet.current_index = 1;
  // pw::draw::DrawSprite(framebuffer,
  //                      sprite_pos_x + 2,
  //                      sprite_pos_y + 2,
  //                      &pigweed_farm_sprite_sheet,
  //                      4);

  // // Draw the farm sprite
  // pigweed_farm_sprite_sheet.current_index = 0;
  // pw::draw::DrawSprite(
  //     framebuffer, sprite_pos_x, sprite_pos_y, &pigweed_farm_sprite_sheet,
  //     4);

  // return 76;
  return 0;
}

// Draw the pigweed text banner.
// Returns the bottom Y coordinate of the bottommost pixel set.
int DrawPigweedBanner(Vector2<int> tl, Framebuffer& framebuffer) {
  constexpr std::array<std::wstring_view, 5> pigweed_banner = {
      L"▒█████▄   █▓  ▄███▒  ▒█    ▒█ ░▓████▒ ░▓████▒ ▒▓████▄",
      L" ▒█░  █░ ░█▒ ██▒ ▀█▒ ▒█░ █ ▒█  ▒█   ▀  ▒█   ▀  ▒█  ▀█▌",
      L" ▒█▄▄▄█░ ░█▒ █▓░ ▄▄░ ▒█░ █ ▒█  ▒███    ▒███    ░█   █▌",
      L" ▒█▀     ░█░ ▓█   █▓ ░█░ █ ▒█  ▒█   ▄  ▒█   ▄  ░█  ▄█▌",
      L" ▒█      ░█░ ░▓███▀   ▒█▓▀▓█░ ░▓████▒ ░▓████▒ ▒▓████▀"};

  auto box_font = pw::draw::GetFont6x8BoxChars();
  // Draw the Pigweed "ASCII" banner.
  for (auto text_row : pigweed_banner) {
    Size<int> string_dims =
        pw::draw::DrawString(text_row,
                             tl,
                             kColorsPico8Rgb565[pw::color::kColorOrange],
                             kBlack,
                             box_font,
                             framebuffer);
    tl.y += string_dims.height;
  }
  return tl.y - box_font.height;
}

// Draw the font sheets.
// Returns the bottom Y coordinate drawn.
int DrawFontSheets(Vector2<int> tl, Framebuffer& framebuffer) {
  constexpr int kFontSheetVerticalPadding = 4;
  constexpr int kFontSheetNumColumns = 26;

  auto font = pw::draw::GetFont6x8BoxChars();
  int initial_x = tl.x;
  tl = DrawColorFontSheet(tl,
                          kFontSheetNumColumns,
                          /*fg_color=*/kBlack,
                          font,
                          framebuffer);

  tl.x = initial_x;
  tl.y -= font.height;
  tl.y += kFontSheetVerticalPadding;

  tl = DrawTestFontSheet(tl,
                         kFontSheetNumColumns,
                         /*fg_color=*/kWhite,
                         /*bg_color=*/kBlack,
                         font,
                         framebuffer);

  tl.x = initial_x;
  tl.y += kFontSheetVerticalPadding;

  Size<int> string_dims = pw::draw::DrawString(L"Box Characters:",
                                               tl,
                                               /*fg_color=*/kWhite,
                                               /*bg_color=*/kBlack,
                                               font,
                                               framebuffer);
  tl.x = 0;

  tl = DrawTestFontSheet(tl,
                         kFontSheetNumColumns,
                         /*fg_color=*/kWhite,
                         /*bg_color=*/kBlack,
                         font,
                         framebuffer);
  return tl.y;
}

// Draw the application header section which is mostly static text/graphics.
// Return the height (in pixels) of the header.
int DrawHeader(Framebuffer& framebuffer) {
  // DrawButton(
  //     g_button, /*bg_color=*/kColorsPico8Rgb565[pw::color::kColorBlue],
  //     framebuffer);
  Vector2<int> tl = {0, 0};
  tl.y = DrawPigweedSprite(framebuffer);

  // tl.y = DrawPigweedBanner(tl, framebuffer);
  constexpr int kFontSheetMargin = 4;
  tl.y += kFontSheetMargin;

  return DrawFontSheets(tl, framebuffer);
}

void DrawLogTextBuffer(int top, const FontSet& font, Framebuffer& framebuffer) {
  constexpr int kLeft = 0;
  Vector2<int> loc;
  Vector2<int> pos{kLeft, top};
  Size<int> buffer_size = s_log_text_buffer.GetSize();
  for (loc.y = 0; loc.y < buffer_size.height; loc.y++) {
    for (loc.x = 0; loc.x < buffer_size.width; loc.x++) {
      auto ch = s_log_text_buffer.GetChar(loc);
      if (!ch.ok())
        continue;
      Size<int> char_size = DrawCharacter(ch->ch,
                                          pos,
                                          ch->foreground_color,
                                          ch->background_color,
                                          font,
                                          framebuffer);
      pos.x += char_size.width;
    }
    pos.y += font.height;
    pos.x = kLeft;
  }
}

void DrawFrame(Framebuffer& framebuffer) {
  constexpr int kHeaderMargin = 4;
  int header_bottom = DrawHeader(framebuffer);
  DrawLogTextBuffer(
      header_bottom + kHeaderMargin, pw::draw::GetFont6x8(), framebuffer);
}

void CreateDemoLogMessages() {
  PW_LOG_CRITICAL("An irrecoverable error has occurred!");
  PW_LOG_ERROR("There was an error on our last operation");
  PW_LOG_WARN("Looks like something is amiss; consider investigating");
  PW_LOG_INFO("The operation went as expected");
  PW_LOG_DEBUG("Debug output");
}

void MainTask(void*) {
  kudzu::FrameCounter frame_counter = kudzu::FrameCounter();

  // TODO(tonymd): Is there a way to hook this up outside of log_basic?
  // pw::log_basic::SetOutput(LogCallback);

  PW_CHECK_OK(Common::Init());

  Display& display = Common::GetDisplay();
  Framebuffer framebuffer = display.GetFramebuffer();
  PW_ASSERT(framebuffer.is_valid());

  pw::draw::Fill(framebuffer, kBlack);

  pw::geometry::Vector3<int> last_frame_touch_state(0, 0, 0);

  DrawFrame(framebuffer);
  // Push the frame buffer to the screen.
  display.ReleaseFramebuffer(std::move(framebuffer));

  // The display loop.
  while (1) {
    frame_counter.StartFrame();

    framebuffer = display.GetFramebuffer();
    PW_ASSERT(framebuffer.is_valid());
    pw::draw::Fill(framebuffer, kBlack);
    DrawFrame(framebuffer);

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
