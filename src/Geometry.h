#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "VAO.h"

#include <glm/glm.hpp>
#include <iostream>

struct Geometry {
  VertexArr VAO;
  std::vector<glm::vec3> vertices;
  std::vector<glm::uvec3> indices;
  glm::vec3 center_of_mass;

  Geometry(std::vector<glm::vec3> v, std::vector<glm::uvec3> in) {
    vertices = v;
    indices = in;
    VAO.vb.bindVertices(vertices);
    VAO.setLayout({3}, false);
    // calculate center of mass
    center_of_mass = glm::vec3(0);
    for (glm::vec3 vert : vertices) {
      center_of_mass += vert;
    }
    center_of_mass /= vertices.size();
  }

  void draw();
  bool intersects(Geometry &g);
  bool intersect_triangles(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
                           glm::vec3 v4, glm::vec3 v5, glm::vec3 v6);
};

#endif