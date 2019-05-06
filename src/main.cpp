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

  Shader fluid_shader("src/shaders/cube.vert", "src/shaders/cube.geom",
                      "src/shaders/cube.frag");

  // create vector of particles
  glm::vec3 min(-1);
  glm::vec3 max(1);
  float radius = 0.05;
  ParticleContainer container(min, max, radius);

  // draw loop
  while (!glfwWindowShouldClose(g.window)) {
    glViewport(0, 0, g.window_width, g.window_height);
    g.applyKeyboardInput();
    g.updateMatrices();
    g.clearRender();

    fluid_shader.use();
    fluid_shader.setMat("projection", g.projection_matrix);
    fluid_shader.setMat("view", g.view_matrix);
    fluid_shader.setMat("model", container.model_matrix);
    fluid_shader.setVec3("light_position", g.light_position);
    fluid_shader.setVec3("camera_position", g.eye);

    container.step_physics(1);
    container.update_instances();
    container.draw();

    g.swapPoll();
  }
  return 0;
}
