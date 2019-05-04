#include <glad/glad.h>

#include "Geometry.h"
#include "Gui.h"
#include "RenderTexture.h"
#include "Scene.h"
#include "Shader.h"
#include "TexturedQuad.h"
#include "VAO.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <string>

int window_width = 640;
int window_height = 480;
std::string window_title = "the simulacrum";

int main(int argc, char *argv[]) {
  // create window/init glfw
  Gui g(window_width, window_height, window_title);
  glfwSetWindowUserPointer(g.window, &g);

  // create scene to contain objects
  Scene scene;

  // solid geometry shader/mesh
  Shader cube_shader("src/shaders/cube.vert", "src/shaders/cube.geom",
                     "src/shaders/cube.frag");
  std::vector<glm::vec3> cube_vertices = {
      {-0.1f, -0.1f, -0.1f}, {-0.1f, -0.1f, 0.1f}, {0.1f, -0.1f, -0.1f},
      {0.1f, -0.1f, 0.1f},   {-0.1f, 0.1f, -0.1f}, {-0.1f, 0.1f, 0.1f},
      {0.1f, 0.1f, -0.1f},   {0.1f, 0.1f, 0.1f}};
  std::vector<glm::uvec3> cube_indices = {
      {0, 2, 1}, {1, 2, 3}, {4, 5, 6}, {5, 7, 6}, {0, 1, 5}, {0, 5, 4},
      {2, 7, 3}, {2, 6, 7}, {3, 5, 1}, {3, 7, 5}, {0, 6, 2}, {0, 4, 6}};
  Geometry cube_geom(cube_vertices, cube_indices);
  scene.add(&cube_geom);

  std::vector<glm::vec3> plane_vertices = {{-2.0f, -1.0f, -2.0f},
                                           {-2.0f, -1.0f, 2.0f},
                                           {2.0, -1.0f, -2.0f},
                                           {2.0f, -1.0f, 2.0f}};
  std::vector<glm::uvec3> plane_indices = {{0, 1, 2}, {1, 3, 2}};
  Geometry plane_geom(plane_vertices, plane_indices);
  plane_geom.dynamic = false;
  scene.add(&plane_geom);

  // draw loop
  while (!glfwWindowShouldClose(g.window)) {
    glViewport(0, 0, g.window_width, g.window_height);
    g.applyKeyboardInput();
    g.updateMatrices();
    g.clearRender();
    scene.intersect_all();

    cube_shader.use();
    cube_shader.setMat("model", g.model_matrix);
    cube_shader.setMat("view", g.view_matrix);
    cube_shader.setMat("projection", g.projection_matrix);
    cube_shader.setVec3("light_position", g.light_position);
    cube_shader.setVec3("camera_position", g.eye);
    cube_geom.draw();
    plane_geom.draw();

    g.swapPoll();
  }
  return 0;
}
