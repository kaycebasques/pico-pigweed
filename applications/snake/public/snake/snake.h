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

#include "pw_assert/assert.h"
#include "pw_containers/inline_deque.h"
#include "pw_function/function.h"

namespace snake {

// Represents the base component of a game object, not necessarily one pixel.
struct Block {
  int32_t x;
  int32_t y;
};

// Represents a moving sequence of blocks.
class Snake {
 public:
  enum Direction {
    kUp,
    kDown,
    kLeft,
    kRight,
  };

  Snake(int32_t max_x, int32_t max_y, size_t initial_size, Direction direction);
  ~Snake();

  // Sets the direction.
  void ChangeDirection(Direction new_direction);

  // Advances one block based on the direction. Sets `ate_fruit to true if the
  // snake // ate with the fruit at x, y. Sets `crashed` to true if the snake
  // collided with its body.
  void Advance(const Block& fruit, bool& ate_fruit, bool& crashed);

  // Returns True if the object at X, Y collides with the snake's body.
  bool CollidesWithBody(const Block& block);

  // Draws the snake on the blank screen by calling a draw callback with the
  // X and Y coordinates of the snake.
  void Draw(const pw::Function<void(const Block&)>& draw_callback);

 private:
  static constexpr size_t kMaxSnakeSize = 25;

  // Returns true if the object at X, Y collides with the snake's head.
  bool CollidesWithHead(const Block& block);

  int32_t AdvanceX(int32_t x) const;
  int32_t AdvanceY(int32_t y) const;

  const int32_t max_x_;
  const int32_t max_y_;
  Direction direction_;
  pw::InlineDeque<Block, kMaxSnakeSize> blocks_;
};

}  // namespace snake
