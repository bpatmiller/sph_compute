#include <glad/glad.h>

#include "Geometry.h"
#include "Gui.h"
#include "RenderTexture.h"
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

  // textured quad for first render pass
  Shader fractal_shader("src/shaders/fractal.vert", "",
                        "src/shaders/fractal.frag");
  TexturedQuad fractal_quad;

  // raw texture
  RenderTexture frame_texture;
  frame_texture.create(g.window_width / g.resolution_scale,
                       g.window_height / g.resolution_scale);

  // fxaa'd texture
  RenderTexture fxaa_texture;
  fxaa_texture.create(g.window_width / g.resolution_scale,
                      g.window_height / g.resolution_scale);

  // fxaa shader
  Shader fxaa_shader("src/shaders/fxaa.vert", "", "src/shaders/fxaa.frag");
  TexturedQuad fxaa_quad;

  // solid geometry shader/mesh
  Shader cube_shader("src/shaders/cube.vert", "", "src/shaders/cube.frag");
  std::vector<glm::vec3> cube_vertices = {
      {-0.1f, -0.1f, -0.1f}, {-0.1f, -0.1f, 0.1f}, {0.1f, -0.1f, -0.1f},
      {0.1f, -0.1f, 0.1f},   {-0.1f, 0.1f, -0.1f}, {-0.1f, 0.1f, 0.1f},
      {0.1f, 0.1f, -0.1f},   {0.1f, 0.1f, 0.1f}};
  std::vector<glm::uvec3> cube_indices = {
      {0, 2, 1}, {1, 2, 3}, {4, 5, 6}, {5, 7, 6}, {0, 1, 5}, {0, 5, 4},
      {2, 7, 3}, {2, 6, 7}, {3, 5, 1}, {3, 7, 5}, {0, 6, 2}, {0, 4, 6}};
  Geometry cube_geom(cube_vertices, cube_indices);

  // draw loop
  while (!glfwWindowShouldClose(g.window)) {
    glViewport(0, 0, g.window_width, g.window_height);

    frame_texture.changeSize((int)g.window_width / g.resolution_scale,
                             (int)g.window_height / g.resolution_scale);
    // g.applyGravity();
    g.applyKeyboardInput();
    g.updateMatrices();
    frame_texture.bind();
    g.clearRender();

    fractal_shader.use();
    fractal_shader.setVec3("camera_position", g.eye);
    fractal_shader.setVec3("fdir", g.fdir);
    fractal_shader.setVec3("sdir", g.sdir);
    fractal_shader.setFloat("aspect", g.aspect);
    fractal_shader.setMat2("rotation1", g.rotation_matrix);
    fractal_shader.setFloat("xres", g.window_width);
    fractal_shader.setFloat("yres", g.window_height);
    fractal_shader.setVec3("light_position", g.light_position);
    fractal_shader.setBool("scary", g.scary);
    fractal_shader.setMat("projection", g.projection_matrix);
    fractal_shader.setMat("view", g.view_matrix);
    fractal_shader.setMat("model", g.model_matrix);
    fractal_quad.draw();
    frame_texture.unbind();

    g.clearRender();

    frame_texture.bindTexture();

    glViewport(0, 0, g.window_width, g.window_height);

    fxaa_shader.use();
    fxaa_shader.setVec2("frameBufSize", g.window_dimensions);
    fxaa_shader.setBool("fxaa", g.fxaa);

    fxaa_shader.setInt("screenTex", 0);
    fxaa_shader.setInt("depTex", 1);
    fxaa_quad.draw();

    frame_texture.unbindTexture();
    /*
    cube_shader.use();
    cube_shader.setMat("projection", g.projection_matrix);
    cube_shader.setMat("view", g.view_matrix);
    cube_shader.setMat("model", g.model_matrix);
    cube_geom.draw();
    */

    g.swapPoll();
  }
  return 0;
}
