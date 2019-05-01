#ifndef TEXTUREDQUAD_H
#define TEXTUREDQUAD_H

#include <glad/glad.h>

#include "Shader.h"
#include "VAO.h"
#include <GLFW/glfw3.h>

class TexturedQuad {
public:
  VertexArr vao;

  std::vector<glm::vec3> vertices = {
      {-1.0f, 1.0f, 0.0f},
      {-1.0f, -1.0f, 0.0f},
      {1.0f, 1.0f, 0.0f},
      {1.0f, -1.0f, 0.0f},
  };

  TexturedQuad() {
    vao.vb.bindVertices(vertices);
    vao.setLayout({3}, false);
  }

  void draw() {
    vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    vao.unbind();
  }
};

#endif