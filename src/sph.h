#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Particle {
  Particle(glm::vec3 p) { position = p; }

  glm::vec3 position;
  float density; // 4
  glm::vec3 velocity;
  float pressure; // 8
  glm::vec3 acceleration;
  float hash; // 12
  glm::vec3 force;
  float _pad0; // 16
  glm::vec3 normal;
  float _pad1; // 20
};

class SPH {
public:
  SPH() { dimensions = glm::vec3(0); };
  SPH(int x, int y, int z) {
    dimensions = glm::vec3(x, y, z);
    num_cells = (x * y * z);
  }
  void init();
  void compute_density();
  void compute_pressure();
  void compute_forces();
  void integrate();
  void step();
  void sort_particles();
  int hash_particle(glm::vec3 _p, float _h, int _n);

  // simulation parameters
  glm::vec3 dimensions;
  int num_cells;
  // particle vector
  std::vector<Particle> particles;
  std::vector<int> hash_to_index_of_first;
  //
  float h = 0.1;

  float MASS = 28.0;
  float REST_DENSITY = 59.0;
  float GAS_CONST = 0.0000001;
  float VISC = 0.8;
  float SURF = 8.0;

  //   float PI = 3.14159265358979323846264338327950288;
  //   float POLY6_COEF = 315.0f / (64 * PI * glm::pow(h, 9));
  //   float POLY6_GRAD_COEF = -945.0f / (32 * PI * glm::pow(h, 9));
  //   float SPIKY_COEF = 15.0f / (PI * glm::pow(h, 6.0f));
  //   float SPIKY_GRAD_COEF = 45.0f / (PI * glm::pow(h, 6));
  //   float VISC_LAPL_COEF = 45.0f / (PI * glm::pow(h, 6));
  //   float C_coef = (32.0f / (PI * glm::pow(h, 9)));
};