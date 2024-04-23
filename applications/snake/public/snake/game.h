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

#include <cstddef>
#include <mutex>

#include "pw_chrono/system_clock.h"
#include "pw_color/colors_pico8.h"
#include "pw_framebuffer/framebuffer.h"
#include "pw_sync/lock_annotations.h"
#include "pw_sync/mutex.h"
#include "pw_touchscreen/buttons.h"
#include "snake/snake.h"

namespace snake {

class Game : public pw::touchscreen::DirectionButtonListener {
 public:
  Game(int32_t screen_width, int32_t screen_height);
  ~Game();

  void Start();
  void Pause();

  void OnButtonUp(bool pressed) override PW_LOCKS_EXCLUDED(lock_) {
    if (!pressed) {
      return;
    }
    std::lock_guard lock(lock_);
    snake_.ChangeDirection(Snake::Direction::kUp);
  }

  void OnButtonDown(bool pressed) override PW_LOCKS_EXCLUDED(lock_) {
    if (!pressed) {
      return;
    }
    std::lock_guard lock(lock_);
    snake_.ChangeDirection(Snake::Direction::kDown);
  }

  void OnButtonLeft(bool pressed) override PW_LOCKS_EXCLUDED(lock_) {
    if (!pressed) {
      return;
    }
    std::lock_guard lock(lock_);
    snake_.ChangeDirection(Snake::Direction::kLeft);
  }

  void OnButtonRight(bool pressed) override PW_LOCKS_EXCLUDED(lock_) {
    if (!pressed) {
      return;
    }
    std::lock_guard lock(lock_);
    snake_.ChangeDirection(Snake::Direction::kRight);
  }

  // Draws screen advancing snake.
  void OnFrame(pw::framebuffer::Framebuffer& framebuffer);

 private:
  static constexpr size_t kMaxSnakeSize = 100;
  static constexpr uint16_t kPixelBoxRatio = 2;

  // Draws the game objects.
  void Draw(pw::framebuffer::Framebuffer& framebuffer) PW_LOCKS_EXCLUDED(lock_);

  void SetNextFruitCoordinates();

  int32_t screen_width_;
  int32_t screen_height_;
  Snake snake_ PW_GUARDED_BY(lock_);
  Block fruit_;
  pw::color::color_rgb565_t fruit_color_;
  bool run_ = false;
  pw::chrono::SystemClock::duration time_per_advance_;
  pw::chrono::SystemClock::time_point last_advance_time_;
  pw::sync::Mutex lock_;
};

}  // namespace snake
