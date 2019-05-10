#pragma once

#include "gl/vao.h"
#include <glm/glm.hpp>
#include <vector>

struct Particle {
  Particle(glm::vec3 p) { position = p; }

  glm::vec3 position;
  float density = 0; // 4
  glm::vec3 velocity = glm::vec3(0);
  float pressure = 0; // 8
  glm::vec3 acceleration = glm::vec3(0);
  float hash = 0; // 12
  glm::vec3 force;
  float _pad0 = 0; // 16 informally defines the index of the first instance
  glm::vec3 normal = glm::vec3(0);
  float _pad1 = 0; // 20 informally defines starting color
};

class SPH {
public:
  SPH() { dimensions = glm::vec3(0); };
  SPH(int x, int y, int z) { dimensions = glm::vec3(x, y, z); }
  void init();
  void sort_particles();
  uint hash_particle(glm::vec3 _p, float _h, int _n);

  // simulation parameters
  glm::vec3 dimensions;
  glm::vec3 box_dimensions;
  float box_scale = 1.0f;
  int num_cells;
  // particle vector
  std::vector<Particle> particles;
  std::vector<int> hash_to_index_of_first;
  VAO accel_vao;
  // simulation constants
  float h = 0.1;
  float timestep = 0.0025f;

  float MASS = 28.0;
  float REST_DENSITY = 59.0;
  float GAS_CONST = 0.0000001;
  float VISC = 1.3;
  float SURF = 18.0;
};