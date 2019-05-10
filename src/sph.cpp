#include "sph.h"
#include <algorithm>
#include <iostream>

void SPH::init() {
  particles.clear();
  // FIXME figure out a better way to tune this feature
  // and adjust the hash function
  num_cells = int((dimensions.x * dimensions.y * dimensions.z) / 8);

  std::cout << "Intilizing SPH with " << num_cells * 8.0 << " particles"
            << std::endl;

  box_dimensions = dimensions * h * box_scale * 0.5f;

  for (int x = 0; x < dimensions.x; x++) {
    for (int y = 0; y < dimensions.y; y++) {
      for (int z = 0; z < dimensions.z; z++) {
        glm::vec3 offset = 0.25f * dimensions * h * (box_scale - 1);
        Particle p(offset + glm::vec3(0.25f * h + 0.5f * h * x,
                                      0.25f * h + 0.5f * h * y,
                                      0.25f * h + 0.5f * h * z));
        p._pad1 = static_cast<float>(x) / dimensions.x;
        particles.emplace_back(p);
      }
    }
  }

  std::cout << "NUM CELLS:" << num_cells << std::endl;

  // initialize 2d array  - hash->particles within that hash
  accel_obj.reserve(num_cells * particles.size() / 8);

  // create VAO
  accel_vao.vb.set(accel_obj);
}

uint SPH::hash_particle(glm::vec3 _p, float _h, int _n) {
  glm::ivec3 p_hat(glm::floor(_p / _h));
  return static_cast<uint>(
      ((p_hat.x * 73856093) ^ (p_hat.y * 19349663) ^ (p_hat.z * 83492791)) %
      _n);
}