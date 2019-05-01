#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtx/io.hpp>
#include <vector>

struct VertexBuf {
  unsigned int vbo;

  VertexBuf() { glGenBuffers(1, &vbo); }

  ~VertexBuf() { glDeleteBuffers(1, &vbo); }

  void bind() { glBindBuffer(GL_ARRAY_BUFFER, vbo); }

  static void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

  void bindVertices(const std::vector<glm::vec3> &vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void bindVertices(const std::vector<glm::vec4> &vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
};

struct VertexArr {
  unsigned int vao;
  VertexBuf vb;
  VertexBuf ib;
  int attCt = 0;

  VertexArr() { glGenVertexArrays(1, &vao); }

  ~VertexArr() { glDeleteVertexArrays(1, &vao); }

  void bind() { glBindVertexArray(vao); }

  void buf_bind(bool instanced) {
    if (instanced) {
      ib.bind();
    } else {
      vb.bind();
    }
  }

  static void unbind() { glBindVertexArray(0); }

  void setLayout(std::initializer_list<uint> sizes, bool instanced) {
    bind();
    buf_bind(instanced);

    int stride = 0;
    size_t offset = 0;
    for (int size : sizes) {
      glEnableVertexAttribArray(attCt);
      glVertexAttribPointer(attCt, size, GL_FLOAT, GL_FALSE,
                            stride * sizeof(float), (void *)offset);
      if (instanced) {
        glVertexAttribDivisor(attCt, 1);
      }
      attCt++;
      offset += size * sizeof(float);
    }

    VertexBuf::unbind();
    unbind();
  }
};

#endif