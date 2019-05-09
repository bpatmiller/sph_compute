#pragma once

#include <glad/glad.h>

#include "gl/program.h"
#include "gl/vao.h"
#include "glm/gtx/rotate_vector.hpp"
#include "sph.h"
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

class Game {
public:
  Game(GLFWwindow *w) { window = w; }
  void init();
  void update();
  void update_camera();
  void create_sphere(float Radius, std::vector<glm::vec3> &s_vertices);

  // window ptr
  GLFWwindow *window;
  // key control
  std::map<int, bool> keyHeld;
  // mouse control
  bool mouse_pressed = false;
  glm::vec2 mouse_pos = glm::vec2(-1, -1);
  glm::vec2 mouse_pos_prev = glm::vec2(-1, -1);
  glm::vec2 mouse_diff = glm::vec2(0, 0);
  // camera properties
  glm::vec3 focus = glm::vec3(0, -0.5, 0);
  glm::vec3 eye = glm::vec3(0, -0.5, 2);
  glm::quat orientation = glm::quat(glm::mat4(1));
  // camera uniforms
  glm::mat4 view_matrix;
  glm::mat4 projection_matrix;

  Program fluid_program;
  VAO fluid;
  SPH simulation;
  std::vector<glm::uvec3> sphere_indices;

  Program pool_program;
  VAO pool;
  std::vector<glm::uvec3> pool_indices;
};