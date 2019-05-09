#include "sph.h"

void SPH::init() {
  for (int x = 0; x < dimensions.x; x++) {
    for (int y = 0; y < dimensions.y; y++) {
      for (int z = 0; z < dimensions.z; z++) {
        particles.emplace_back(Particle(glm::vec3(h * x, h * y, h * z)));
      }
    }
  }
}