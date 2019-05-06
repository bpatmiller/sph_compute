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
  Shader fluid_shader("src/shaders/cube.vert", "src/shaders/cube.geom",
                      "src/shaders/cube.frag");
  glm::vec3 min(-1);
  glm::vec3 max(1);
  float radius = 0.1;
  ParticleContainer container(min, max, radius);

  // pool shader/object
  Shader pool_shader("src/shaders/pool.vert", "", "src/shaders/pool.frag");
  std::vector<glm::vec3> pool_vertices = {
      {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
  std::vector<glm::uvec3> pool_indices = {
      {0, 1, 2}, {1, 3, 2}, {4, 6, 5}, {5, 6, 7}, {0, 5, 1}, {0, 4, 5},
      {2, 3, 7}, {2, 7, 6}, {3, 1, 5}, {3, 5, 7}, {0, 2, 6}, {0, 6, 4}};
  Geometry pool_geometry(pool_vertices, pool_indices);

  // draw loop
  while (!glfwWindowShouldClose(g.window)) {
    glViewport(0, 0, g.window_width, g.window_height);
    g.applyKeyboardInput();
    g.updateMatrices();
    g.clearRender();

    // pool pass
    pool_shader.use();
    pool_shader.setMat("projection", g.projection_matrix);
    pool_shader.setMat("view", g.view_matrix);
    pool_shader.setMat("model", container.model_matrix);
    pool_shader.setVec3("light_position", g.light_position);
    pool_shader.setVec3("camera_position", g.eye);

    pool_geometry.draw();

    // fluid pass
    fluid_shader.use();
    fluid_shader.setMat("projection", g.projection_matrix);
    fluid_shader.setMat("view", g.view_matrix);
    fluid_shader.setMat("model", container.model_matrix);
    fluid_shader.setVec3("light_position", g.light_position);
    fluid_shader.setVec3("camera_position", g.eye);
    fluid_shader.setMat("inverse_rotation", g.inverse_rotation);

    container.step_physics(1);
    container.update_instances();
    container.draw();

    g.swapPoll();
  }
  return 0;
}
