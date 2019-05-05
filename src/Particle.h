#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <vector>

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 force;
  float pressure;
  std::vector<Particle *> neighbors;

  Particle(glm::vec3 p) {
    position = p;
    pressure = 0;
  }

  Particle() {
    position = glm::vec3(0);
    pressure = 0;
  }
};

#endif