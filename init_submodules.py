# Just a convenience script for properly initializing Git submodules
# because it never seems to work correctly for me...

import os

submodules = [
    ("third_party/pcb/esp-rust-board", "https://github.com/esp-rs/esp-rust-board"),
    ("third_party/pcb/gekkio-kicad-libs", "https://github.com/Gekkio/gekkio-kicad-libs"),
    ("third_party/pigweed-experimental", "https://pigweed.googlesource.com/pigweed/experimental"),
    ("third_party/pigweed", "https://pigweed.googlesource.com/pigweed/pigweed"),
    ("third_party/freertos", "https://pigweed.googlesource.com/third_party/github/FreeRTOS/FreeRTOS-Kernel.git"),
    ("third_party/pcb/espressif-kicad-libraries", "https://github.com/espressif/kicad-libraries"),
    ("third_party/32blit-sdk", "https://github.com/32blit/32blit-sdk"),
    ("third_party/pico-sdk", "https://pigweed.googlesource.com/third_party/github/raspberrypi/pico-sdk")
]

for submodule in submodules:
    command = f'git submodule add {submodule[1]} {submodule[0]}'
    os.system(command)
