// Copyright 2022 The Pigweed Authors
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
#include <cstdint>

#include "app_common/common.h"

#define LIB_CMSIS_CORE 0
#define LIB_PICO_STDIO_SEMIHOSTING 0

#include "FreeRTOS.h"
#include "ft6236/device.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/vreg.h"
#include "icm42670p/device.h"
#include "kudzu_buttons_pi4ioe5v6416/buttons.h"
#include "kudzu_imu_icm42670p/imu.h"
#include "max17048/device.h"
#include "pi4ioe5v6416/device.h"
#include "pico/stdlib.h"
#include "pw_digital_io_rp2040/digital_io.h"
#include "pw_i2c_rp2040/initiator.h"
#include "pw_log/log.h"
#include "pw_pixel_pusher_rp2040_pio/pixel_pusher.h"
#include "pw_spi/chip_selector_digital_out.h"
#include "pw_spi_rp2040/initiator.h"
#include "pw_status/status.h"
#include "pw_sync/borrow.h"
#include "pw_sync/mutex.h"
#include "pw_thread/detached_thread.h"
#include "pw_thread/thread.h"
#include "pw_thread_freertos/context.h"
#include "pw_thread_freertos/options.h"
#include "pw_touchscreen_ft6236/touchscreen.h"

#if defined(DISPLAY_TYPE_ILI9341)
#include "pw_display_driver_ili9341/display_driver.h"
using DisplayDriver = pw::display_driver::DisplayDriverILI9341;
#elif defined(DISPLAY_TYPE_ST7735)
#include "pw_display_driver_st7735/display_driver.h"
using DisplayDriver = pw::display_driver::DisplayDriverST7735;
#elif defined(DISPLAY_TYPE_ST7789)
#include "pw_display_driver_st7789/display_driver.h"
using DisplayDriver = pw::display_driver::DisplayDriverST7789;
#elif defined(DISPLAY_TYPE_ST7789_PIO)
#include "pw_display_driver_st7789/display_driver.h"
using DisplayDriver = pw::display_driver::DisplayDriverST7789;
#else
#error "Undefined display type"
#endif

using Touchscreen = pw::touchscreen::TouchscreenFT6236;
using Buttons = kudzu::ButtonsPI4IOE5V6416;

using pw::Status;
using pw::digital_io::Rp2040Config;
using pw::digital_io::Rp2040DigitalIn;
using pw::digital_io::Rp2040DigitalInOut;
using pw::display::Display;
using pw::framebuffer::Framebuffer;
using pw::framebuffer::PixelFormat;
using pw::framebuffer_pool::FramebufferPool;
using pw::pixel_pusher::PixelPusherRp2040Pio;
using pw::spi::Device;
using pw::spi::DigitalOutChipSelector;
using pw::spi::Initiator;
using pw::spi::Rp2040Initiator;
using pw::sync::Borrowable;
using pw::sync::VirtualMutex;

namespace {

// Pico spi0 Pins
#define SPI_PORT spi0

struct SpiValues {
  SpiValues(pw::spi::Config config,
            pw::spi::ChipSelector& selector,
            pw::sync::VirtualMutex& initiator_mutex);

  pw::spi::Rp2040Initiator initiator;
  pw::sync::Borrowable<pw::spi::Initiator> borrowable_initiator;
  pw::spi::Device device;
};

static_assert(DISPLAY_WIDTH > 0);
static_assert(DISPLAY_HEIGHT > 0);

constexpr uint16_t kDisplayScaleFactor = 2;
constexpr uint16_t kFramebufferWidth =
    FRAMEBUFFER_WIDTH >= 0 ? FRAMEBUFFER_WIDTH / kDisplayScaleFactor
                           : DISPLAY_WIDTH / kDisplayScaleFactor;
constexpr uint16_t kFramebufferHeight = DISPLAY_HEIGHT / kDisplayScaleFactor;

constexpr pw::geometry::Size<uint16_t> kDisplaySize{DISPLAY_WIDTH,
                                                    DISPLAY_HEIGHT};
constexpr size_t kNumPixels = kFramebufferWidth * kFramebufferHeight;
constexpr uint16_t kFramebufferRowBytes = sizeof(uint16_t) * kFramebufferWidth;

constexpr uint32_t kBaudRate = 31'250'000;
constexpr pw::spi::Config kSpiConfig8Bit{
    .polarity = pw::spi::ClockPolarity::kActiveHigh,
    .phase = pw::spi::ClockPhase::kRisingEdge,
    .bits_per_word = pw::spi::BitsPerWord(8),
    .bit_order = pw::spi::BitOrder::kMsbFirst,
};
constexpr pw::spi::Config kSpiConfig16Bit{
    .polarity = pw::spi::ClockPolarity::kActiveHigh,
    .phase = pw::spi::ClockPhase::kRisingEdge,
    .bits_per_word = pw::spi::BitsPerWord(16),
    .bit_order = pw::spi::BitOrder::kMsbFirst,
};

Rp2040DigitalInOut s_display_dc_pin({
    .pin = DISPLAY_DC_GPIO,
    .polarity = pw::digital_io::Polarity::kActiveHigh,
});
#if DISPLAY_RESET_GPIO != -1
Rp2040DigitalInOut s_display_reset_pin({
    .pin = DISPLAY_RESET_GPIO,
    .polarity = pw::digital_io::Polarity::kActiveLow,
});
#endif
#if DISPLAY_TE_GPIO != -1
Rp2040DigitalIn s_display_tear_effect_pin({
    .pin = DISPLAY_TE_GPIO,
    .polarity = pw::digital_io::Polarity::kActiveHigh,
});
#endif
Rp2040DigitalInOut s_display_cs_pin({
    .pin = DISPLAY_CS_GPIO,
    .polarity = pw::digital_io::Polarity::kActiveLow,
});
DigitalOutChipSelector s_spi_chip_selector(s_display_cs_pin);
Rp2040Initiator s_spi_initiator(SPI_PORT);
VirtualMutex s_spi_initiator_mutex;
Borrowable<Initiator> s_borrowable_spi_initiator(s_spi_initiator,
                                                 s_spi_initiator_mutex);
SpiValues s_spi_8_bit(kSpiConfig8Bit,
                      s_spi_chip_selector,
                      s_spi_initiator_mutex);
SpiValues s_spi_16_bit(kSpiConfig16Bit,
                       s_spi_chip_selector,
                       s_spi_initiator_mutex);

#if USE_PIO
PixelPusherRp2040Pio s_pixel_pusher(DISPLAY_DC_GPIO,
                                    DISPLAY_CS_GPIO,
                                    SPI_MOSI_GPIO,
                                    SPI_CLOCK_GPIO,
                                    DISPLAY_TE_GPIO,
                                    pio0);
#endif
uint16_t s_pixel_data1[kNumPixels];
uint16_t s_pixel_data2[kNumPixels];
const pw::Vector<void*, 2> s_pixel_buffers{s_pixel_data1, s_pixel_data2};
pw::framebuffer_pool::FramebufferPool s_fb_pool({
    .fb_addr = s_pixel_buffers,
    .dimensions = {kFramebufferWidth, kFramebufferHeight},
    .row_bytes = kFramebufferRowBytes,
    .pixel_format = PixelFormat::RGB565,
});
DisplayDriver s_display_driver({
    .data_cmd_gpio = s_display_dc_pin.as<pw::digital_io::DigitalOut>(),
    .spi_cs_gpio = s_display_cs_pin.as<pw::digital_io::DigitalOut>(),
#if DISPLAY_RESET_GPIO != -1
    .reset_gpio = &s_display_reset_pin.as<pw::digital_io::DigitalOut>(),
#else
    .reset_gpio = nullptr,
#endif
#if DISPLAY_TE_GPIO != -1
    .tear_effect_gpio = &s_display_tear_effect_pin,
#else
    .tear_effect_gpio = nullptr,
#endif
    .spi_device_8_bit = s_spi_8_bit.device,
    .spi_device_16_bit = s_spi_16_bit.device,
#if USE_PIO
    .pixel_pusher = &s_pixel_pusher,
#endif
});

#if BACKLIGHT_GPIO != -1
void SetBacklight(uint16_t brightness) {
  pwm_config cfg = pwm_get_default_config();
  pwm_set_wrap(pwm_gpio_to_slice_num(BACKLIGHT_GPIO), 65535);
  pwm_init(pwm_gpio_to_slice_num(BACKLIGHT_GPIO), &cfg, true);
  gpio_set_function(BACKLIGHT_GPIO, GPIO_FUNC_PWM);

  pwm_set_gpio_level(BACKLIGHT_GPIO, brightness);
}
#endif

const uint8_t kStatusPinRed = 23;
const uint8_t kStatusPinGreen = 24;
const uint8_t kStatusPinBlue = 25;

void ConfigStatusRgb() {
  std::array<uint8_t, 3> pwm_pins = {
      kStatusPinRed, kStatusPinGreen, kStatusPinBlue};
  for (auto& pin : pwm_pins) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    auto slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config cfg = pwm_get_default_config();
    pwm_set_wrap(slice_num, 65535);
    pwm_init(slice_num, &cfg, true);

    pwm_set_gpio_level(pin, 65535);
  }
}

void SetStatusRgb(uint8_t r, uint8_t g, uint8_t b) {
  pwm_set_gpio_level(kStatusPinRed, 65535 - r * r);
  pwm_set_gpio_level(kStatusPinGreen, 65535 - g * g);
  pwm_set_gpio_level(kStatusPinBlue, 65535 - b * b);
}

SpiValues::SpiValues(pw::spi::Config config,
                     pw::spi::ChipSelector& selector,
                     pw::sync::VirtualMutex& initiator_mutex)
    : initiator(SPI_PORT),
      borrowable_initiator(initiator, initiator_mutex),
      device(borrowable_initiator, config, selector) {}

constexpr pw::i2c::Rp2040Initiator::Config ki2c0Config{
    .clock_frequency = 400'000,
    .sda_pin = I2C_BUS0_SDA,
    .scl_pin = I2C_BUS0_SCL,
};
constexpr pw::i2c::Rp2040Initiator::Config ki2c1Config{
    .clock_frequency = 400'000,
    .sda_pin = I2C_BUS1_SDA,
    .scl_pin = I2C_BUS1_SCL,
};
pw::i2c::Rp2040Initiator i2c0_bus(ki2c0Config, i2c0);
pw::i2c::Rp2040Initiator i2c1_bus(ki2c1Config, i2c1);

pw::pi4ioe5v6416::Device io_expander(i2c1_bus);
kudzu::icm42670p::Device imu(i2c0_bus);
pw::max17048::Device fuel_guage(i2c0_bus);
pw::ft6236::Device touch_screen_controller(i2c0_bus);

static constexpr size_t kDisplayDrawThreadStackWords = 512;
static pw::thread::freertos::StaticContextWithStack<
    kDisplayDrawThreadStackWords>
    display_draw_thread_context;

static constexpr size_t kTouchscreenThreadStackWords = 512;
static pw::thread::freertos::StaticContextWithStack<
    kTouchscreenThreadStackWords>
    touchscreen_thread_context;

Rp2040DigitalInOut s_io_reset_n({
    .pin = 10,
    // IO expander resets when this pin is pulled low.
    .polarity = pw::digital_io::Polarity::kActiveLow,
});
Rp2040DigitalInOut s_io_interrupt_n({
    .pin = 11,
    .polarity = pw::digital_io::Polarity::kActiveLow,
});
Rp2040DigitalInOut s_imu_fsync({
    .pin = 13,
    .polarity = pw::digital_io::Polarity::kActiveHigh,
});

}  // namespace

Status Common::EndOfFrameCallback() {
  touch_screen_controller.LogControllerInfo();

  if (io_expander.Probe() == pw::OkStatus()) {
    io_expander.LogControllerInfo();
  }

  if (fuel_guage.Probe() == pw::OkStatus()) {
    fuel_guage.LogControllerInfo();
  }

  if (imu.Probe() == pw::OkStatus()) {
    imu.LogControllerInfo();
  }

  return pw::OkStatus();
}

// static
Status Common::Init() {
#if OVERCLOCK_250
  // Overvolt for a stable 250MHz on some RP2040s
  vreg_set_voltage(VREG_VOLTAGE_1_20);
  sleep_ms(10);
  set_sys_clock_khz(250000, false);
#endif

  adc_init();

  ConfigStatusRgb();
  // Set to a dim pink.
  SetStatusRgb(32, 12, 32);

  s_display_cs_pin.Enable();
  s_display_dc_pin.Enable();
#if DISPLAY_RESET_GPIO != -1
  s_display_reset_pin.Enable();
#endif

#if DISPLAY_TE_GPIO != -1
  s_display_tear_effect_pin.Enable();
#endif

  i2c0_bus.Enable();
  i2c1_bus.Enable();

  s_io_reset_n.Enable();
  // Disable reset pin - normal operation.
  s_io_reset_n.SetStateInactive();
  s_io_interrupt_n.Enable();

  // IMU FSYNC not used yet
  s_imu_fsync.Enable();

  touch_screen_controller.Enable();
  if (touch_screen_controller.Probe() == pw::OkStatus()) {
    touch_screen_controller.LogControllerInfo();
  }

  io_expander.Enable();
  if (io_expander.Probe() == pw::OkStatus()) {
    io_expander.LogControllerInfo();
  }

  fuel_guage.Enable();
  if (fuel_guage.Probe() == pw::OkStatus()) {
    fuel_guage.LogControllerInfo();
  }

  imu.Enable();
  if (imu.Probe() == pw::OkStatus()) {
    imu.LogControllerInfo();
  }

#if BACKLIGHT_GPIO != -1
  SetBacklight(0xffff);  // Full brightness.
#endif

  unsigned actual_baudrate = spi_init(SPI_PORT, kBaudRate);
  PW_LOG_DEBUG("Actual Baudrate: %u", actual_baudrate);

#if SPI_MISO_GPIO != -1
  gpio_set_function(SPI_MISO_GPIO, GPIO_FUNC_SPI);
#endif
  gpio_set_function(SPI_CLOCK_GPIO, GPIO_FUNC_SPI);
  gpio_set_function(SPI_MOSI_GPIO, GPIO_FUNC_SPI);

#if USE_PIO
  // Init the display before the pixel pusher.
  s_display_driver.Init();
  auto result = s_pixel_pusher.Init(s_fb_pool);
  s_pixel_pusher.SetPixelDouble(true);
  return result;
#else
  return s_display_driver.Init();
#endif
}

// static
pw::display::Display& Common::GetDisplay() {
  static Display s_display(s_display_driver, kDisplaySize, s_fb_pool);
  return s_display;
}

pw::touchscreen::Touchscreen& Common::GetTouchscreen() {
  static Touchscreen s_touchscreen = Touchscreen(&touch_screen_controller);
  return s_touchscreen;
}

kudzu::Buttons& Common::GetButtons() {
  static Buttons s_buttons = Buttons(&io_expander);
  return s_buttons;
}

kudzu::imu::PollingImu& Common::GetImu() {
  static kudzu::imu::PollingImuICM42670P s_imu(&imu);
  return s_imu;
}

const pw::thread::Options& Common::DisplayDrawThreadOptions() {
  static constexpr auto options =
      pw::thread::freertos::Options()
          .set_name("DisplayDrawThread")
          .set_static_context(display_draw_thread_context)
          // TODO: amontanez - Find a way to better manage priorities.
          .set_priority(static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1));
  return options;
}

const pw::thread::Options& Common::TouchscreenThreadOptions() {
  static constexpr auto options =
      pw::thread::freertos::Options()
          .set_name("TouchscreenThread")
          .set_static_context(touchscreen_thread_context)
          // TODO: amontanez - Find a way to better manage priorities.
          .set_priority(static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1));
  return options;
}
