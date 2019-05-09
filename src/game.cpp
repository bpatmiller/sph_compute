#include "game.h"
#include <algorithm>

void Game::create_sphere(float Radius, std::vector<glm::vec3> &s_vertices) {
  int Stacks = 5;
  int Slices = 5;
  s_vertices.clear();

  for (int i = 0; i <= Stacks; ++i) {
    float V = i / (float)Stacks;
    float phi = V * glm::pi<float>();
    for (int j = 0; j <= Slices; ++j) {
      float U = j / (float)Slices;
      float theta = U * (glm::pi<float>() * 2);
      float x = cosf(theta) * sinf(phi);
      float y = cosf(phi);
      float z = sinf(theta) * sinf(phi);
      s_vertices.push_back(glm::vec3(x, y, z) * Radius);
    }
  }

  for (int i = 0; i < Slices * Stacks + Slices; ++i) {
    sphere_indices.emplace_back(glm::uvec3(i, i + Slices + 1, i + Slices));
    sphere_indices.emplace_back(glm::uvec3(i + Slices + 1, i, i + 1));
  }
}

void Game::init() {
  simulation.dimensions = glm::vec3(20, 20, 20);
  simulation.h = 0.1;

  // compile programs
  pool_program = Program("src/shaders/pool.vert", "src/shaders/pool.geom",
                         "src/shaders/pool.frag", "");
  fluid_program =
      Program("src/shaders/cube.vert", "", "src/shaders/cube.frag", "");
  fluid_compute_dens = Program("", "", "", "src/shaders/sph_dens_pres.glsl");

  // init pool
  std::vector<glm::vec3> pool_vertices = {
      {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f},
      {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
  for (auto &v : pool_vertices) {
    v.x *= simulation.dimensions.x * simulation.h * 1.0f;
    v.y *= simulation.dimensions.y * simulation.h * 1.0f;
    v.z *= simulation.dimensions.z * simulation.h * 1.0f;
  }
  pool_indices = {{0, 1, 2}, {1, 3, 2}, {0, 5, 1}, {0, 4, 5}, {2, 3, 7},
                  {2, 7, 6}, {3, 1, 5}, {3, 5, 7}, {0, 2, 6}, {0, 6, 4}};
  pool.setLayout({3}, false);
  pool.vb.set(pool_vertices);

  // init particles
  simulation.init();

  // init sphere, pass particle instances
  std::vector<glm::vec3> sphere_vertices;
  create_sphere(simulation.h * 0.25f, sphere_vertices);
  fluid.setLayout({3}, false);
  fluid.setLayout({3, 1, 3, 1, 3, 1, 3, 1, 3, 1}, true);
  fluid.vb.set(sphere_vertices);
  fluid.ib.set(simulation.particles);

  // partical SSBO for compute shader
  glGenBuffers(1, &fluid_ssbo_id);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fluid_ssbo_id);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fluid.ib.id);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // spatial acceleration SSBO
  glGenBuffers(1, &accel_ssbo_id);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, accel_ssbo_id);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, simulation.accel_vao.vb.id);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 1);
}

void Game::update() {
  // check window size
  int window_width, window_height;
  glfwGetWindowSize(window, &window_width, &window_height);

  // pass camera uniforms
  view_matrix =
      glm::translate(glm::mat4(1.0f), -eye) * glm::mat4_cast(orientation);
  projection_matrix = glm::perspective(
      glm::radians(80.0f), ((float)window_width) / window_height, 0.01f, 20.f);

  // clear render, set params
  glViewport(0, 0, window_width, window_height);
  glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LESS);
  glCullFace(GL_BACK);

  // handle mouse movement
  if (mouse_pressed) {
    bool first = (mouse_pos_prev == glm::vec2(-1, -1));
    if (first) {
      mouse_pos_prev = mouse_pos;
    } else {
      mouse_diff = mouse_pos - mouse_pos_prev;
      update_camera();
      mouse_pos_prev = mouse_pos;
    }
  } else {
    mouse_pos_prev = mouse_pos;
  }

  // handle keypress
  if (keyHeld[GLFW_KEY_W]) {
    eye -= 0.01f * glm::normalize(eye - focus);
  }
  if (keyHeld[GLFW_KEY_S]) {
    eye += 0.01f * glm::normalize(eye - focus);
  }

  // render the pool
  pool_program.use();
  pool_program.setMat4("projection", projection_matrix);
  pool_program.setMat4("view", view_matrix);
  pool.bind();
  glDrawElements(GL_TRIANGLES, pool_indices.size() * 3, GL_UNSIGNED_INT,
                 pool_indices.data());

  // sort particles and poulate a map of index -> index pairs
  simulation.sort_particles();
  fluid.ib.update(simulation.particles, 0);

  // step the fluids
  fluid_compute_dens.use();
  fluid_compute_dens.setFloat("time", glfwGetTime());
  fluid_compute_dens.setInt("particles_size", simulation.particles.size());
  fluid_compute_dens.setInt("num_cells", simulation.num_cells);
  fluid_compute_dens.setFloat("MASS", simulation.MASS);
  fluid_compute_dens.setFloat("GAS_CONST", simulation.GAS_CONST);
  fluid_compute_dens.setFloat("REST_DENS", simulation.REST_DENSITY);
  fluid_compute_dens.setFloat("h", simulation.h);
  glDispatchCompute(simulation.particles.size(), 1, 1);

  // render the fluids
  fluid_program.use();
  fluid_program.setMat4("projection", projection_matrix);
  fluid_program.setMat4("view", view_matrix);
  fluid_program.setInt("color_mode", color_mode);
  fluid_program.setInt("num_cells", simulation.num_cells);
  fluid.bind();
  glDrawElementsInstanced(GL_TRIANGLES, sphere_indices.size() * 3,
                          GL_UNSIGNED_INT, sphere_indices.data(),
                          simulation.particles.size());
}

void Game::update_camera() {
  if (glm::length(mouse_diff) == 0)
    return;
  mouse_diff *= 0.1f;
  glm::quat qyaw = glm::angleAxis(glm::radians(mouse_diff.y), SIDE);
  glm::quat qpitch = glm::angleAxis(glm::radians(mouse_diff.x), UP);
  orientation = qyaw * orientation * qpitch;
}