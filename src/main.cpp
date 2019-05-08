#include <glad/glad.h>

#include "Gui.h"
#include "Particle.h"
#include "ParticleContainer.h"
#include "RenderTexture.h"
#include "Scene.h"
#include "Shader.h"
#include "TexturedQuad.h"
#include "VAO.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

int window_width = 640;
int window_height = 480;
std::string window_title = "the simulacrum";

int main(int argc, char *argv[]) {
  // create window/init glfw
  Gui g(window_width, window_height, window_title);
  glfwSetWindowUserPointer(g.window, &g);

  // particle shader/object
  Shader fluid_shader("src/shaders/cube.vert", "", "src/shaders/cube.frag", "");
  // Shader fluid_compute_shader("", "", "", "src/shaders/fluid_comp.glsl");

  //
  glm::vec3 min(-0.9, 0.0, -0.45);
  glm::vec3 max(0.0, 1.0, 0.45);
  glm::vec3 container_min(-1.0, -1.0, -0.5);
  glm::vec3 container_max(1.0, 1.0, 0.5);

  float scale = 3.0f;
  min *= scale;
  max *= scale;
  container_min *= scale;
  container_max *= scale;
  ParticleContainer container(min, max, container_min, container_max);
  //

  // pool shader/object
  Shader pool_shader("src/shaders/pool.vert", "src/shaders/pool.geom",
                     "src/shaders/pool.frag", "");
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
  std::vector<glm::uvec3> pool_indices = {
      {0, 1, 2}, {1, 3, 2}, {0, 5, 1}, {0, 4, 5}, {2, 3, 7},
      {2, 7, 6}, {3, 1, 5}, {3, 5, 7}, {0, 2, 6}, {0, 6, 4}};
  Geometry pool_geometry(pool_vertices, pool_indices);

  // draw loop
  while (!glfwWindowShouldClose(g.window)) {
    glViewport(0, 0, g.window_width, g.window_height);
    g.applyKeyboardInput();
    g.updateMatrices();
    g.clearRender();

    // pool pass

    pool_shader.use();
    pool_shader.setMat4("projection", g.projection_matrix);
    pool_shader.setMat4("view", g.view_matrix);

    pool_geometry.draw();

    // fluid pass
    fluid_shader.use();
    fluid_shader.setMat4("projection", g.projection_matrix);
    fluid_shader.setMat4("view", g.view_matrix);
    container.draw();

    container.step_physics(5);
    container.update_instances();

    g.swapPoll();
  }
  return 0;
}
