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

#pragma once

#include <stdint.h>

#include "pw_chrono/system_clock.h"
#include "pw_ring_buffer/prefixed_entry_ring_buffer.h"

namespace kudzu {

class FrameCounter {
 public:
  FrameCounter();

  void StartFrame();
  void EndDraw();
  void EndFlush();
  void LogTiming();
  inline pw::chrono::SystemClock::duration LastFrameDuration() {
    return last_frame_duration;
  }
  inline std::chrono::milliseconds LastFrameMilliseconds() {
    return std::chrono::round<std::chrono::milliseconds>(last_frame_duration);
  }

 private:
  pw::chrono::SystemClock::time_point second_counter_start;
  // Start of a single frame / start of the draw phase.
  pw::chrono::SystemClock::time_point frame_start;
  // End of the draw phase / start of the flush phase.
  pw::chrono::SystemClock::time_point draw_end;
  // End of the flush phase and end of the frame.
  pw::chrono::SystemClock::time_point frame_end;
  // Duration of frame_start to frame_end.
  pw::chrono::SystemClock::duration last_frame_duration;
  uint32_t frame_count;
  int frames_per_second;
  std::byte draw_buffer[30 * sizeof(uint32_t)];
  std::byte flush_buffer[30 * sizeof(uint32_t)];
  pw::ring_buffer::PrefixedEntryRingBuffer draw_times;
  pw::ring_buffer::PrefixedEntryRingBuffer flush_times;
};

/// Given a ring buffer full of uint32_t values, return the average value or
/// zero if empty (or iteration error).
uint32_t CalcAverageUint32Value(
    pw::ring_buffer::PrefixedEntryRingBuffer& ring_buffer);

}  // namespace kudzu
