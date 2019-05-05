#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  float radius;
  float pressure;

  Particle(glm::vec3 p) { position = p; }
};

#endif