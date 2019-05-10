#include "sph.h"
#include <algorithm>
#include <iostream>

void SPH::init() {
  particles.clear();
  // FIXME figure out a better way to tune this feature
  // and adjust the hash function
  num_cells = int((dimensions.x * dimensions.y * dimensions.z) / 1.0);

  std::cout << "Intilizing SPH with " << num_cells << " particles" << std::endl;

  box_dimensions = dimensions * h * box_scale * 1.0f;
  glm::vec3 offset = (0.5f * dimensions * h * (box_scale - 1)) + glm::vec3(0.25f * h);

  for (int x = 0; x < dimensions.x; x++) {
    for (int y = 0; y < dimensions.y; y++) {
      for (int z = 0; z < dimensions.z; z++) {
        float jitterx = 0.01f* static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float jittery = 0.01f* static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float jitterz = 0.01f* static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        Particle p(offset + glm::vec3(h * x + jitterx,
                                       h * y + jittery,
                                       h * z + jitterz));
        p._pad1 = static_cast<float>(x) / dimensions.x;
        particles.emplace_back(p);
      }
    }
  }

  // resize hash vector
  hash_to_index_of_first.resize(num_cells);

  // create VAO
  accel_vao.setLayout({1}, false);
  accel_vao.vb.set(hash_to_index_of_first);
}

uint SPH::hash_particle(glm::vec3 _p, float _h, int _n) {
  glm::ivec3 p_hat(glm::floor(_p / _h));
  return static_cast<uint>(
      ((p_hat.x * 73856093) ^ (p_hat.y * 19349663) ^ (p_hat.z * 83492791)) %
      _n);
}

void SPH::sort_particles() {
  // hash each particle
  // for (auto &p : particles) {
  //   p.hash = hash_particle(p.position, h, num_cells);
  // }
  // sort particles by hash
  auto hash_compare = [](Particle &a, Particle &b) { return a.hash < b.hash; };
  std::sort(particles.begin(), particles.end(), hash_compare);
  std::fill(hash_to_index_of_first.begin(), hash_to_index_of_first.end(), 0);
  // for each particle, if the hash is new
  // add the index of that particle to hash_to_first_index
  // so that a block with a given hash can access the first
  // instance of blocks with the same hash
  int current_firstseen_hash = -1;
  int current_firstseen_index = -1;
  for (uint i = 0; i < particles.size(); i++) {
    // when you see a new key, add the new value to the index
    // you see it in
    if (particles[i].hash != current_firstseen_hash) {
      current_firstseen_hash = particles[i].hash;
      hash_to_index_of_first[current_firstseen_hash] = i;
    }
  }
  // FIXME maybe update VAO
  accel_vao.vb.update(hash_to_index_of_first, 0);
}