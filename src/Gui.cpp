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
  // if (rotation_angle > 1.0)
  //  rotation_angle = 0;
  // rotation_angle += 0.0001;
  light_position = eye + 0.01f * fdir;
  rotation_matrix[0][0] = glm::cos(rotation_angle);
  rotation_matrix[0][1] = glm::sin(rotation_angle);
  rotation_matrix[1][0] = -rotation_matrix[0][1];
  rotation_matrix[1][1] = rotation_matrix[0][0];

  aspect = static_cast<float>(window_width) / window_height;

  window_dimensions.x = window_width;
  window_dimensions.y = window_height;
  // screenRes = glm::vec3(window_width, window_height, 0.0f);
  glm::mat4 translate = glm::mat4(1.0f);
  translate = glm::translate(translate, -eye);
  view_matrix = glm::mat4_cast(orientation) * translate;
  projection_matrix = glm::perspective(
      glm::radians(80.0f), ((float)window_width) / window_height, 0.1f, 20.f);
  model_matrix = glm::mat4(1.0f);
}

float Gui::DE(glm::vec3 w) {
  int iterations = 32;
  int bailout = 10.0;
  float scale = 1.9;

  float r;
  float x = w.x;
  float y = w.y;
  float z = w.z;
  float x1, y1, z1;
  int i;

  r = x * x + y * y + z * z;
  for (i = 0; i < iterations && r < bailout; i++) {

    // rotate(x,y,z)
    float x2 = x * rotation_matrix[0][0] + y * rotation_matrix[1][0];
    float y2 = x * rotation_matrix[0][1] + y * rotation_matrix[1][1];

    x = x2;
    y = y2;

    if (x + y < 0) {
      x1 = -y;
      y = -x;
      x = x1;
    }
    if (x + z < 0) {
      x1 = -z;
      z = -x;
      x = x1;
    }
    if (y + z < 0) {
      y1 = -z;
      z = -y;
      y = y1;
    }

    y2 = y * rotation_matrix[0][0] + z * rotation_matrix[1][0];
    float z2 = y * rotation_matrix[0][1] + z * rotation_matrix[1][1];

    y = y2;
    z = z2;

    x = scale * x - (scale - 1);
    y = scale * y - (scale - 1);
    z = scale * z - (scale - 1);
    r = x * x + y * y + z * z;
  }
  return (sqrt(r) - 2) * pow(scale, -i);
}

void Gui::applyGravity() {
  float player_height = 0.025f;
  float EPS = 0.015f;

  on_ground = false;

  float eye_d = DE(eye);

  if (eye_d < EPS) {
    on_ground = true;
    eye += UP * 0.01f;
  } else if (eye_d < player_height) {
    on_ground = true;
  }
  if (!on_ground) {
    momentum.y -= 0.001;
    eye += momentum.y * UP;
  } else {
    momentum.y = 0;
  }
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
  if (key == GLFW_KEY_Q)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  else if (key == GLFW_KEY_F && action != GLFW_RELEASE) {
    fxaa = !fxaa;
    // std::cout << "fxaa: " << fxaa << std::endl;
  } else if (key == GLFW_KEY_G && action != GLFW_RELEASE) {
    scary = !scary;
  } else if (key == GLFW_KEY_EQUAL && action != GLFW_RELEASE) {
    if (resolution_scale < 16)
      resolution_scale += 1;
  } else if (key == GLFW_KEY_MINUS && action != GLFW_RELEASE) {
    if (resolution_scale > 2)
      resolution_scale -= 1;
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
  if (keyHeld[GLFW_KEY_SPACE]) {
    if (on_ground) {
      momentum.y += 0.01;
      on_ground = false;
      eye += momentum.y * UP;
    }
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
