#include "Gui.h"

#include "glm/gtx/rotate_vector.hpp"
#include <glm/glm.hpp>

void Gui::clearRender() {
  glfwGetFramebufferSize(window, &window_width, &window_height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LESS);
  glCullFace(GL_BACK);
}

void Gui::updateMatrices() {
  aspect = static_cast<float>(window_width) / window_height;
  window_dimensions.x = window_width;
  window_dimensions.y = window_height;

  rotation = glm::mat4_cast(orientation);
  inverse_rotation = glm::inverse(rotation);
  glm::mat4 translate = glm::mat4(1.0f);
  translate = glm::translate(translate, -eye);
  view_matrix = rotation * translate;
  projection_matrix = glm::perspective(
      glm::radians(80.0f), ((float)window_width) / window_height, 0.01f, 20.f);
}

void Gui::swapPoll() {
  glfwSwapBuffers(window);
  glfwPollEvents();
}

void Gui::mouseButtonCallback(int button, int action, int mods) {
  if (current_x <= window_width)
    mouse_pressed = (action == GLFW_PRESS);
}

void Gui::mousePosCallback(double mouse_x, double mouse_y) {
  bool first = (current_x == -1 || current_y == -1);
  float dx = mouse_x - current_x;
  float dy = mouse_y - current_y;
  current_x = mouse_x;
  current_y = mouse_y;
  if (first)
    return;

  dx *= 0.5;
  dy *= 0.5;

  glm::quat qyaw = glm::angleAxis(glm::radians(dy), SIDE);
  glm::quat qpitch = glm::angleAxis(glm::radians(dx), UP);
  orientation = qyaw * orientation * qpitch;

  glm::mat4 inv = glm::inverse(glm::mat4_cast(orientation));

  fdir = glm::vec3(inv * glm::vec4(FORWARD, 1));
  sdir = glm::vec3(inv * glm::vec4(SIDE, 1));
  updir = glm::cross(fdir, sdir);
}

void Gui::keyCallback(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_Q) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    return;
  }
  if (action == GLFW_PRESS) {
    keyHeld[key] = true;
  } else if (action == GLFW_RELEASE) {
    keyHeld[key] = false;
  }
}

void Gui::applyKeyboardInput() {
  float move_speed = 0.01f;
  if (keyHeld[GLFW_KEY_W]) {
    eye += fdir * move_speed;
  }
  if (keyHeld[GLFW_KEY_S]) {
    eye -= fdir * move_speed;
  }
  if (keyHeld[GLFW_KEY_A]) {
    eye -= sdir * move_speed;
  }
  if (keyHeld[GLFW_KEY_D]) {
    eye += sdir * move_speed;
  }
}

void Gui::MouseButtonCallback(GLFWwindow *window, int button, int action,
                              int mods) {
  Gui *gui = (Gui *)glfwGetWindowUserPointer(window);
  gui->mouseButtonCallback(button, action, mods);
}

void Gui::MousePosCallback(GLFWwindow *window, double mouse_x, double mouse_y) {
  Gui *gui = (Gui *)glfwGetWindowUserPointer(window);
  gui->mousePosCallback(mouse_x, mouse_y);
}

void Gui::KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                      int mods) {
  Gui *gui = (Gui *)glfwGetWindowUserPointer(window);
  gui->keyCallback(key, scancode, action, mods);
}
