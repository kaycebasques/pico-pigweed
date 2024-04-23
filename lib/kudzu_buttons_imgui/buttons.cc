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

#include "kudzu_buttons_imgui/buttons.h"

#include "kudzu_buttons/buttons.h"

#define PW_LOG_MODULE_NAME "kudzu_buttons_imgui"
#define PW_LOG_LEVEL PW_LOG_LEVEL_INFO

#include "pw_display_driver_imgui/display_driver.h"
#include "pw_log/log.h"
#include "pw_status/status.h"

namespace kudzu {

namespace {

std::bitset<kButtonCount> current_buttons;

// GLFW keyboard event handler.
// See also: https://www.glfw.org/docs/3.3/input_guide.html#input_key
void key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods) {
  bool state;
  if (action == GLFW_PRESS) {
    state = true;
  } else if (action == GLFW_RELEASE) {
    state = false;
  } else {
    // Don't handle actions other than press or release.
    return;
  }

  // GLFW Keycodes: https://www.glfw.org/docs/3.3/group__keys.html
  if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
    current_buttons[kudzu::button::up] = state;
  } else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
    current_buttons[kudzu::button::left] = state;
  } else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
    current_buttons[kudzu::button::down] = state;
  } else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
    current_buttons[kudzu::button::right] = state;
  } else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_C) {
    current_buttons[kudzu::button::start] = state;
  } else if (key == GLFW_KEY_TAB || key == GLFW_KEY_V) {
    current_buttons[kudzu::button::select] = state;
  } else if (key == GLFW_KEY_COMMA || key == GLFW_KEY_Z) {
    current_buttons[kudzu::button::b] = state;
  } else if (key == GLFW_KEY_PERIOD || key == GLFW_KEY_X) {
    current_buttons[kudzu::button::a] = state;
  }
}

}  // namespace

ButtonsImgui::ButtonsImgui(
    pw::display_driver::DisplayDriverImgUI& display_driver)
    : display_driver_(display_driver) {}

pw::Status ButtonsImgui::Init() {
  glfwSetKeyCallback(display_driver_.GetGlfwWindow(), key_callback);
  current_buttons.reset();

  return pw::OkStatus();
}

pw::Result<std::bitset<kButtonCount>> ButtonsImgui::DoUpdate() {
  return current_buttons;
}

}  // namespace kudzu
