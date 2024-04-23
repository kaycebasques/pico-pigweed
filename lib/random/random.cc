#include "libkudzu/random.h"

#include <stdint.h>

#include "pw_random/random.h"
#include "pw_random/xor_shift.h"

namespace {

constexpr uint64_t kRandomSeed = 314159265358979;
static pw::random::XorShiftStarRng64 rng(kRandomSeed);

uint32_t random_seed_offset = 0;
uint32_t current_random_seed = 0x64063701;

}  // namespace

uint32_t GetCurrentSeed() { return current_random_seed; }

void RestartSeed() {
  rng = pw::random::XorShiftStarRng64(kRandomSeed + random_seed_offset);
}

void IncrementSeed(int diff) {
  current_random_seed += diff;
  random_seed_offset += diff;
  RestartSeed();
}

void SetSeed(uint32_t seed) {
  current_random_seed = seed;
  RestartSeed();
}

uint32_t GetRandomNumber() {
  int random_value = 0;
  rng.GetInt(random_value);
  return (uint32_t)random_value;
}

int GetRandomInteger(uint32_t max_value) {
  return (int)(GetRandomNumber() % max_value);
}

int GetRandomInteger(uint32_t min_value, uint32_t max_value) {
  int diff = max_value - min_value;
  if (diff < 0)
    diff *= -1;

  int r = GetRandomNumber() % (uint32_t)(diff);
  r += min_value;

  return r;
}

float GetRandomFloat(float max_value) {
  uint32_t r = GetRandomNumber() % (uint32_t)(max_value);
  uint32_t d = GetRandomNumber() % 1000000;
  float decimal_part = (float)d / 1000000.0f;
  float x = (float)r + decimal_part;
  return x;
}

float GetRandomFloat(float min_value, float max_value) {
  float diff = max_value - min_value;
  float r = GetRandomFloat(diff);
  r += min_value;
  return r;
}
