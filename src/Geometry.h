#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "VAO.h"

#include <glm/glm.hpp>
#include <iostream>

struct Geometry {
  VertexArr VAO;
  std::vector<glm::vec3> vertices;
  std::vector<glm::uvec3> indices;
  glm::fquat orientation;
  glm::mat4 translation;
  glm::vec3 center_of_mass;
  bool dirty;

  Geometry(std::vector<glm::vec3> v, std::vector<glm::uvec3> in) {
    vertices = v;
    indices = in;
    orientation = glm::quat(glm::mat4(1));
    translation = glm::mat4(1);
    VAO.vb.bindVertices(vertices);
    VAO.setLayout({3}, false);
    // calculate center of mass
    center_of_mass = glm::vec3(0);
    for (glm::vec3 vert : vertices) {
      center_of_mass += vert;
    }
    center_of_mass /= vertices.size();
    dirty = true;
  }

  void draw();
  float vec2_cross(glm::vec2 v1, glm::vec2 v2);
  bool edge_intersect(std::vector<glm::vec2> e1, std::vector<glm::vec2> e2);
  bool intersects(Geometry &g);
  glm::vec4 intersect_triangles(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
                                glm::vec3 v4, glm::vec3 v5, glm::vec3 v6);
};

#endif