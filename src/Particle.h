#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <vector>

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 acceleration;
  glm::vec3 acceleration_old;
  glm::vec3 force;
  glm::vec3 normal;
  float density;
  float pressure;
  std::vector<Particle *> neighbors;

  Particle(glm::vec3 p) {
    position = p;
    pressure = 0;
    density = 0;
    neighbors.clear();
    acceleration_old = glm::vec3(0);
  }
};

#endif