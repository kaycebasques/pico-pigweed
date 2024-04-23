#pragma once

#include <cinttypes>

#include "pw_color/color.h"
#include "pw_draw/sprite_sheet.h"

using pw::color::color_rgb565_t;

// Pixel data for the Pigweed logo.
const color_rgb565_t pw_logo5x7_sprite_data[] = {

    // Sprite 0
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xd43f,  // #d787ff
    0xf81f,  // #ff00ff
    0xfebf,  // #ffd7ff
    0xf81f,  // #ff00ff
    0xd43f,  // #d787ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xfebf,  // #ffd7ff
    0xd43f,  // #d787ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xfebf,  // #ffd7ff
    0xaaff,  // #af5fff
    0xaaff,  // #af5fff
    0xf81f,  // #ff00ff
    0x6b0,   // #00d787
    0xfebf,  // #ffd7ff
    0xaaff,  // #af5fff
    0xf81f,  // #ff00ff
    0x6b0,   // #00d787
    0xf81f,  // #ff00ff
    0x6b0,   // #00d787
    0x6b0,   // #00d787
    0x6b0,   // #00d787
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0x6b0,   // #00d787
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff

};

pw::draw::SpriteSheet pw_logo5x7_sprite_sheet = {
    .width = 5,
    .height = 7,
    .count = 1,
    .transparent_color = 0xf81f,
    ._data = pw_logo5x7_sprite_data};
