#pragma once

#include <glad/glad.h>

#include "gl/program.h"
#include "gl/rendertexture.h"
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
  glm::vec3 screenspace_to_world(glm::vec2 coords, glm::vec4 viewport);
  bool intersect_sphere(glm::vec3 ray_direction, glm::vec3 p);
  void mouse_ray_intersect();
  float intersect_plane(glm::vec3 ray_direction);
  void toggle_render_mode() {
    std::cout << "toggle render mode" << std::endl;
    render_mode++;
    if (render_mode > 3)
      render_mode = 1;
  };

  // window ptr
  GLFWwindow *window;
  int window_width, window_height;
  // key control
  std::map<int, bool> keyHeld;
  // mouse control
  bool mouse_pressed = false;
  glm::vec2 mouse_pos = glm::vec2(-1, -1);
  glm::vec2 mouse_pos_prev = glm::vec2(-1, -1);
  glm::vec2 mouse_diff = glm::vec2(0, 0);
  // camera properties
  glm::vec3 focus = glm::vec3(0, 0, 0);
  glm::vec3 base_eye = glm::vec3(0, 0, 4);
  glm::vec3 eye = glm::vec3(0, 0, 4);
  float yaw = 0;
  float pitch = 0;
  glm::quat orientation = glm::quat(glm::mat4(1));
  // camera uniforms
  glm::mat4 view_matrix;
  glm::mat4 projection_matrix;

  int color_mode = 9;
  int render_mode = 1;
  float attract_repel = 1.0f;
  bool pipe = false;
  bool law = false;
  glm::vec3 repulser = glm::vec3(-99, -99, -99);

  // fluid programs/ssbo/indices/SPH object
  Program fluid_program;
  Program fluid_compute_dens;
  Program fluid_compute_norm_vel;
  Program fluid_compute_force;
  Program fluid_integrate;
  GLuint fluid_ssbo_id = 0;
  GLuint accel_ssbo_id = 0;
  VAO fluid;
  SPH simulation;
  std::vector<int> hash_to_index_of_first;
  std::vector<glm::uvec3> sphere_indices;
  uint PHYSICS_STEPS = 10;

  // pool program/vao
  Program pool_program;
  VAO pool;
  std::vector<glm::uvec3> pool_indices;

  // texture quad vertices and vao
  RenderTexture r_tex;

  Program tex_quad_program;
  VAO texquad;
  std::vector<glm::vec3> tq_vertices = {
      {-1.0f, 1.0f, 0.0f},
      {-1.0f, -1.0f, 0.0f},
      {1.0f, 1.0f, 0.0f},
      {1.0f, -1.0f, 0.0f},
  };

  // ssao kernel
  uint ssao_kernel_size = 20;
  std::vector<glm::vec3> ssao_kernel;
};
