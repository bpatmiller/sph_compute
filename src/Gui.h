#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>

#include "VAO.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <map>
#include <string>

const glm::vec3 FORWARD(0, 0, -1);
const glm::vec3 SIDE(1, 0, 0);
const glm::vec3 UP(0, 1, 0);

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

class Gui {
public:
  GLFWwindow *window;

  std::string window_title;
  int window_width;
  int window_height;
  float aspect;
  glm::vec3 screenRes;

  std::map<int, bool> keyHeld;

  glm::vec2 window_dimensions = glm::vec2(1, 1);

  int current_x = -1;
  int current_y = -1;
  bool mouse_pressed = false;
  bool on_ground = false;
  bool flying = false;

  glm::vec3 light_position = glm::vec3(-1.0f, 4.0f, 0.7f);

  // camera properties
  glm::vec3 eye = glm::vec3(0.0, 0.0, 4.5);
  glm::quat orientation = glm::quat(glm::mat4(1.0f));
  glm::vec3 fdir = FORWARD;
  glm::vec3 sdir = SIDE;
  glm::vec3 updir = UP;
  glm::vec3 gfdir = fdir;
  glm::vec3 gsdir = sdir;
  // camera uniforms
  glm::mat4 view_matrix;
  glm::mat4 projection_matrix;

  Gui(int w, int h, std::string title) {
    window_width = w;
    window_height = h;
    window_title = title;
    // init glfw
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
      exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(window_width, window_height, window_title.c_str(),
                              NULL, NULL);
    if (!window) {
      glfwTerminate();
      std::cout << "error" << std::endl;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);
    // set input callbacks
    glfwSetCursorPosCallback(window, MousePosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    updateMatrices();
  }

  ~Gui() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  float DE(glm::vec3 w);
  void applyGravity();

  void swapPoll();
  void clearRender();
  void updateMatrices();
  void applyKeyboardInput();

  void mouseButtonCallback(int button, int action, int mods);
  void mousePosCallback(double mouse_x, double mouse_y);
  void keyCallback(int key, int scancode, int action, int mods);
  static void MouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void MousePosCallback(GLFWwindow *window, double mouse_x,
                               double mouse_y);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
};

#endif
