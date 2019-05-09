#include "sph.h"

void SPH::init() {
  particles.clear();
  for (int x = 0; x < dimensions.x; x++) {
    for (int y = 0; y < dimensions.y; y++) {
      for (int z = 0; z < dimensions.z; z++) {
        particles.emplace_back(Particle(glm::vec3(0.25f * h + 0.5f * h * x,
                                                  0.25f * h + 0.5f * h * y,
                                                  0.25f * h + 0.5f * h * z)));
      }
    }
  }
}