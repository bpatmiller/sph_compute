#include "VAO.h"

#include <glm/glm.hpp>

struct Geometry {
  VertexArr VAO;
  std::vector<glm::vec3> vertices;
  std::vector<glm::uvec3> indices;

  Geometry(std::vector<glm::vec3> v, std::vector<glm::uvec3> in) {
    vertices = v;
    indices = in;
    VAO.vb.bindVertices(vertices);
    VAO.setLayout({3}, false);
  }

  void draw() {
    VAO.bind();
    glDrawElements(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT,
                   indices.data());
  }
};
