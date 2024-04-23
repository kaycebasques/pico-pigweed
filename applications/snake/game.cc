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
#include "snake/game.h"

#include <cstddef>
#include <cstdint>
#include <mutex>

#include "libkudzu/random.h"
#include "pw_assert/check.h"
#include "pw_chrono/system_clock.h"
#include "pw_color/colors_pico8.h"
#include "pw_draw/draw.h"
#include "pw_log/log.h"

using namespace std::chrono_literals;

namespace snake {

constexpr auto kDefaultTimePerAdvance =
    pw::chrono::SystemClock::for_at_least(60ms);

Game::Game(int32_t screen_width, int32_t screen_height)
    : screen_width_(screen_width / kPixelBoxRatio),
      screen_height_(screen_height / kPixelBoxRatio),
      snake_(screen_width_,
             screen_height_,
             /*initial_size=*/3 * kPixelBoxRatio,
             Snake::Direction::kLeft),
      fruit_({.x = 0, .y = 0}),
      fruit_color_(pw::color::kColorsPico8Rgb565[pw::color::kColorBlue]),
      run_(false),
      time_per_advance_(kDefaultTimePerAdvance),
      last_advance_time_(pw::chrono::SystemClock::now()) {
  SetNextFruitCoordinates();
}

Game::~Game() {}

void Game::Start() { run_ = true; }

void Game::Pause() { run_ = false; }

void Game::OnFrame(pw::framebuffer::Framebuffer& framebuffer) {
  if (!run_) {
    Draw(framebuffer);
    return;
  }
  bool crashed = false;
  bool ate_fruit = false;
  auto current_time = pw::chrono::SystemClock::now();
  if (current_time > last_advance_time_ + time_per_advance_) {
    std::lock_guard lock(lock_);
    snake_.Advance(fruit_, ate_fruit, crashed);
    last_advance_time_ = current_time;
  }
  if (crashed) {
    // TODO(cachinchilla): draw a crash.
    PW_CHECK(false);
    return;
  }

  if (ate_fruit) {
    time_per_advance_ -= pw::chrono::SystemClock::for_at_least(5ms);
    if (time_per_advance_.count() <= 0) {
      time_per_advance_ = pw::chrono::SystemClock::for_at_least(0ms);
    }
    SetNextFruitCoordinates();
  }
  Draw(framebuffer);
}

void Game::Draw(pw::framebuffer::Framebuffer& framebuffer) {
  // Draw Fruit.
  pw::draw::DrawRect(framebuffer,
                     kPixelBoxRatio * fruit_.x,
                     kPixelBoxRatio * fruit_.y,
                     kPixelBoxRatio * fruit_.x + kPixelBoxRatio,
                     kPixelBoxRatio * fruit_.y + kPixelBoxRatio,
                     fruit_color_,
                     /*filled=*/true);

  // Draw Snake.
  std::lock_guard lock(lock_);
  snake_.Draw([&framebuffer](const Block& block) {
    pw::draw::DrawRect(framebuffer,
                       kPixelBoxRatio * block.x,
                       kPixelBoxRatio * block.y,
                       kPixelBoxRatio * block.x + kPixelBoxRatio,
                       kPixelBoxRatio * block.y + kPixelBoxRatio,
                       pw::color::kColorsPico8Rgb565[pw::color::kColorGreen],
                       /*filled=*/true);
  });
}

void Game::SetNextFruitCoordinates() {
  std::lock_guard lock(lock_);
  do {
    fruit_.x = GetRandomInteger(screen_width_);
    fruit_.y = GetRandomInteger(screen_height_);
  } while (snake_.CollidesWithBody(fruit_));
  PW_LOG_DEBUG("Drawing fruit at x=%d y=%d",
               static_cast<int>(fruit_.x),
               static_cast<int>(fruit_.y));
}

}  // namespace snake
