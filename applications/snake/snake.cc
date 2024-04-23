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
#include "snake/snake.h"

#include "pw_assert/check.h"

namespace snake {
namespace {

constexpr size_t CyclicDecrement(size_t value, size_t max_value) {
  return value >= 1 ? value - 1 : max_value;
}

constexpr size_t CyclicIncrement(size_t value, size_t max_value) {
  return (value + 1) % (max_value + 1);
}

constexpr bool DoBlocksCollide(const Block& block, const Block& other_block) {
  return block.x == other_block.x && block.y == other_block.y;
}

}  // namespace

Snake::Snake(int32_t max_x,
             int32_t max_y,
             size_t initial_size,
             Direction direction)
    : max_x_(max_x), max_y_(max_y), direction_(direction) {
  // Initial size cannot be wider than screen width.
  PW_CHECK(static_cast<int>(initial_size) < max_x_);
  PW_CHECK(initial_size > 0);

  const int32_t y = max_y_ / 2;
  int32_t x = (max_x_ - static_cast<int>(initial_size)) / 2;
  for (size_t i = 0; i < initial_size; ++i) {
    Block block{.x = x, .y = y};
    blocks_.push_back(std::move(block));
    x = CyclicIncrement(x, max_x_);
  }
}

Snake::~Snake() {}

void Snake::ChangeDirection(Direction new_direction) {
  if (direction_ == new_direction ||
      (direction_ == Direction::kUp && new_direction == Direction::kDown) ||
      (direction_ == Direction::kDown && new_direction == Direction::kUp) ||
      (direction_ == Direction::kLeft && new_direction == Direction::kRight) ||
      (direction_ == Direction::kRight && new_direction == Direction::kLeft)) {
    return;
  }
  direction_ = new_direction;
}

void Snake::Advance(const Block& fruit, bool& ate_fruit, bool& crashed) {
  crashed = false;
  ate_fruit = false;
  Block next_head{.x = AdvanceX(blocks_.front().x),
                  .y = AdvanceY(blocks_.front().y)};
  if (CollidesWithBody(next_head)) {
    // Snake crashed!
    crashed = true;
    return;
  }

  ate_fruit = DoBlocksCollide(fruit, next_head);
  // Advance head.
  blocks_.push_front(next_head);

  // Advance tail if the object was not eaten or the snake is its max size.
  if (!ate_fruit || blocks_.full()) {
    blocks_.pop_back();
  }
}

int32_t Snake::AdvanceX(int32_t x) const {
  switch (direction_) {
    case kLeft:
      return CyclicDecrement(x, max_x_);
    case kRight:
      return CyclicIncrement(x, max_x_);
    default:
      return x;
  };
}

int32_t Snake::AdvanceY(int32_t y) const {
  switch (direction_) {
    case kUp:
      return CyclicDecrement(y, max_y_);
    case kDown:
      return CyclicIncrement(y, max_y_);
    default:
      return y;
  };
}

bool Snake::CollidesWithBody(const Block& other_block) {
  for (const auto& block : blocks_) {
    if (DoBlocksCollide(block, other_block)) {
      return true;
    }
  }
  return false;
}

void Snake::Draw(const pw::Function<void(const Block&)>& draw_callback) {
  for (const auto& block : blocks_) {
    // Draw block given x and  y;
    draw_callback(block);
  }
}

}  // namespace snake
