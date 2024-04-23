#pragma once

#include <cinttypes>

#include "pw_color/color.h"
#include "pw_draw/sprite_sheet.h"

using pw::color::color_rgb565_t;

// Pixel data for a small red heart.
const color_rgb565_t heart_8x8_sprite_data[] = {

    // Sprite 0
    0xf81f,  // #ff00ff
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf81f,  // #ff00ff
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf81f,  // #ff00ff
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf81f,  // #ff00ff
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf800,  // #ff0000
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf800,  // #ff0000
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff
    0xf81f,  // #ff00ff

};

pw::draw::SpriteSheet heart_8x8_sprite_sheet = {.width = 8,
                                                .height = 8,
                                                .count = 1,
                                                .transparent_color = 0xf81f,
                                                ._data = heart_8x8_sprite_data};
