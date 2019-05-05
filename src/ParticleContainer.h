#ifndef PARTICLECONTAINER_H
#define PARTICLECONTAINER_H

#include "Particle.h"
#include "VAO.h"
#include <glm/gtx/transform.hpp>
#include <vector>

struct ParticleContainer {
  VertexArr VAO;
  std::vector<glm::vec3> vertices;
  std::vector<glm::uvec3> face_indices;
  glm::mat4 model_matrix;

  std::vector<Particle> particles;
  std::vector<glm::vec3> positions;

  ParticleContainer(glm::vec3 min, glm::vec3 max, float step, float radius) {
    create_sphere(radius);

    // generate initial positions
    for (float x = min.x; x <= max.x; x += step) {
      for (float y = min.y; y <= max.y; y += step) {
        for (float z = min.z; z <= max.z; z += step) {
          Particle p = Particle(glm::vec3(x, y, z));
          p.radius = radius;
          particles.emplace_back(p);
        }
      }
    }
    // update instance data
    update_instances();
    // bind vao
    VAO.vb.bindVertices(vertices);
    VAO.ib.bindVertices(positions);
    VAO.setLayout({3}, false);
    VAO.setLayout({3}, true);
    // define cube scale
    // model_matrix = glm::scale(glm::vec3(0.05));
    model_matrix = glm::mat4(1.0f);
  }
  void draw();
  void update_instances();
  void create_sphere(float Radius);
};

#endif