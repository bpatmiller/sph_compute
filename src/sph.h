#pragma once

#include <glm/glm.hpp>
#include <vector>

const float h = 0.1;

const float MASS = 28.0;
const float REST_DENSITY = 59.0;
const float GAS_CONST = 0.0000001;
const float VISC = 0.8;
const float SURF = 8.0;

const float PI = 3.14159265358979323846264338327950288;
const float POLY6_COEF = 315.0f / (64 * PI * glm::pow(h, 9));
const float POLY6_GRAD_COEF = -945.0f / (32 * PI * glm::pow(h, 9));
const float SPIKY_COEF = 15.0f / (PI * glm::pow(h, 6.0f));
const float SPIKY_GRAD_COEF = 45.0f / (PI * glm::pow(h, 6));
const float VISC_LAPL_COEF = 45.0f / (PI * glm::pow(h, 6));
const float C_coef = (32.0f / (PI * glm::pow(h, 9)));

struct Particle {
  Particle(glm::vec3 p) { position = p; }

  glm::vec3 position;
  float density; // 4
  glm::vec3 velocity;
  float pressure; // 8
  glm::vec3 acceleration;
  float color; // 12
  glm::vec3 force;
  float _pad0; // 16
  glm::vec3 normal;
  float _pad1; // 20
};

class SPH {
public:
  SPH() { dimensions = glm::ivec3(0); };
  SPH(int x, int y, int z) { dimensions = glm::ivec3(x, y, z); }
  void init();
  void compute_density();
  void compute_pressure();
  void compute_forces();
  void integrate();
  void step();

  float size;
  // simulation parameters
  glm::ivec3 dimensions;
  // particle vector
  std::vector<Particle> particles;
};