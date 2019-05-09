#include "game.h"

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
  pool_program = Program("src/shaders/pool.vert", "src/shaders/pool.geom",
                         "src/shaders/pool.frag", "");
  fluid_program =
      Program("src/shaders/cube.vert", "", "src/shaders/cube.frag", "");

  // define particle container
  glm::vec3 min(-0.9, 0.0, -0.45);
  glm::vec3 max(0.0, 1.0, 0.45);
  glm::vec3 container_min(-1.0, -1.0, -0.5);
  glm::vec3 container_max(1.0, 1.0, 0.5);

  // init pool
  std::vector<glm::vec3> pool_vertices = {
      {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, 1.0f},   {-1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, -1.0f},   {1.0f, 1.0f, 1.0f}};
  // fit the pool to container size
  for (uint i = 0; i < pool_vertices.size(); i++) {
    for (int k = 0; k < 3; k++) {
      if (pool_vertices[i][k] == -1.0f) {
        pool_vertices[i][k] = container_min[k];
      } else {
        pool_vertices[i][k] = container_max[k];
      }
    }
  }
  pool.setLayout({3}, false);
  pool.vb.set(pool_vertices);
  pool_indices = {{0, 1, 2}, {1, 3, 2}, {0, 5, 1}, {0, 4, 5}, {2, 3, 7},
                  {2, 7, 6}, {3, 1, 5}, {3, 5, 7}, {0, 2, 6}, {0, 6, 4}};

  // init particles
  simulation.dimensions = glm::ivec3(20, 50, 20);
  simulation.init();

  std::vector<glm::vec3> sphere_vertices;
  create_sphere(0.025, sphere_vertices);
  fluid.setLayout({3}, false);
  fluid.setLayout({3, 1, 3, 1, 3, 1, 3, 1, 3, 1}, true);
  fluid.vb.set(sphere_vertices);
  fluid.ib.set(simulation.particles);
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

  // render the pool
  pool_program.use();
  pool_program.setMat4("projection", projection_matrix);
  pool_program.setMat4("view", view_matrix);
  pool.bind();
  glDrawElements(GL_TRIANGLES, pool_indices.size() * 3, GL_UNSIGNED_INT,
                 pool_indices.data());

  fluid_program.use();
  fluid_program.setMat4("projection", projection_matrix);
  fluid_program.setMat4("view", view_matrix);
  fluid.bind();
  glDrawElementsInstanced(GL_TRIANGLES, sphere_indices.size() * 3,
                          GL_UNSIGNED_INT, sphere_indices.data(),
                          simulation.particles.size());
  // glDrawElements(GL_TRIANGLES, sphere_indices.size() * 3, GL_UNSIGNED_INT,
  //                sphere_indices.data());
}

void Game::update_camera() {
  if (glm::length(mouse_diff) == 0)
    return;
  mouse_diff *= 0.1f;
  glm::quat qyaw = glm::angleAxis(glm::radians(mouse_diff.y), SIDE);
  glm::quat qpitch = glm::angleAxis(glm::radians(mouse_diff.x), UP);
  orientation = qyaw * orientation * qpitch;
}