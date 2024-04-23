// Copyright 2024 The Pigweed Authors
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

#include "libkudzu/framecounter.h"

#include <chrono>

#define PW_LOG_LEVEL PW_LOG_LEVEL_DEBUG
#define PW_LOG_MODULE_NAME "FrameCounter"

#include <cstdint>
#include <cstring>

#include "pw_chrono/system_clock.h"
#include "pw_log/log.h"
#include "pw_ring_buffer/prefixed_entry_ring_buffer.h"

using namespace std::chrono_literals;

namespace kudzu {

FrameCounter::FrameCounter() {
  second_counter_start = pw::chrono::SystemClock::now();
  frame_count = 0;
  frames_per_second = 0;
  draw_times.SetBuffer(draw_buffer);
  flush_times.SetBuffer(flush_buffer);
}

void FrameCounter::StartFrame() {
  frame_start = pw::chrono::SystemClock::now();
}

void FrameCounter::EndDraw() {
  draw_end = pw::chrono::SystemClock::now();
  auto draw_duration = draw_end - frame_start;
  uint32_t elapsed_millis =
      std::chrono::round<std::chrono::microseconds>(draw_duration).count();
  draw_times.PushBack(
      pw::as_bytes(pw::span{std::addressof(elapsed_millis), 1}));
}

void FrameCounter::EndFlush() {
  frame_end = pw::chrono::SystemClock::now();
  last_frame_duration = frame_end - frame_start;
  auto flush_duration = frame_end - draw_end;
  frame_count++;
  uint32_t elapsed_millis =
      std::chrono::round<std::chrono::microseconds>(flush_duration).count();
  flush_times.PushBack(
      pw::as_bytes(pw::span{std::addressof(elapsed_millis), 1}));
}

void FrameCounter::LogTiming() {
  if (frame_end - second_counter_start >
      pw::chrono::SystemClock::for_at_least(1000ms)) {
    frames_per_second = frame_count;
    frame_count = 0;
    PW_LOG_INFO("FPS:%d, Draw:%dus, Flush:%dus",
                frames_per_second,
                (int)CalcAverageUint32Value(draw_times),
                (int)CalcAverageUint32Value(flush_times));
    second_counter_start = pw::chrono::SystemClock::now();
  }
}

uint32_t CalcAverageUint32Value(
    pw::ring_buffer::PrefixedEntryRingBuffer& ring_buffer) {
  uint64_t sum = 0;
  uint32_t count = 0;
  for (const auto& entry_info : ring_buffer) {
    PW_ASSERT(entry_info.buffer.size() == sizeof(uint32_t));
    uint32_t val;
    std::memcpy(&val, entry_info.buffer.data(), sizeof(val));
    sum += val;
    count++;
  }
  return count == 0 ? 0 : sum / count;
}

}  // namespace kudzu
