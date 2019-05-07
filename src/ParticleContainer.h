#ifndef PARTICLECONTAINER_H
#define PARTICLECONTAINER_H

#include "Particle.h"
#include "VAO.h"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>

struct ParticleContainer {
  // render objects
  VertexArr VAO;
  glm::mat4 model_matrix;
  std::vector<glm::vec3> vertices;
  // instance vector
  std::vector<glm::vec4> positions;
  // particles and particle - hash map
  std::vector<Particle> particles;
  std::unordered_multimap<int, Particle *> block_hashmap;
  // simulation parameters
  float timestep = 0.01;
  float particle_radius = 0.1;
  float h = particle_radius * 4; 
  // fluid coefficients
  float mass = 28.0;// m
  float rest_density = 59.0;// rho_0
  float gas_constant = 0.000001;//k
  float viscosity = 1.3;//mu
  float surface_tension_coef = 8.0;//yotta  
  // container/tuning parameters
  float container_width = 1;
  float container_depth = 1;
  uint max_neighbors = 16;
  float stiffness = 7.0;
  float damping = 0.25;
  glm::vec3 g = glm::vec3(0,-9.8,0);
  // math coefficients
  const float PI = 3.14159265358979323846264338327950288;
  const float poly6_coef = 315 / (64 * PI * glm::pow(h, 9));
  const float poly6_grad_coef = 945 / (32 * PI * glm::pow(h, 9));
  const float spiky_grad_coef = 45 / (PI * glm::pow(h, 6));
  const float laplacian_visc_coef = 45 / (PI * glm::pow(h, 6));
  const float C_coef = (32 / (PI * glm::pow(h, 9)));

  ParticleContainer(glm::vec3 min, glm::vec3 max, float radius) {
    // load mesh
    create_sprite(particle_radius);

    // generate initial positions
    for (float x = min.x; x <= max.x; x += radius * 0.5f) {
      for (float y = min.y; y <= max.y; y += radius * 0.5f) {
        for (float z = min.z; z <= max.z; z += radius *0.5f) {
          Particle p = Particle(glm::vec3(x, y, z));
          particles.emplace_back(p);
          positions.emplace_back(glm::vec4(p.position, p.density));
        }
      }
    }

    // bind vao
    VAO.vb.bindVertices(vertices);
    VAO.ib.bindVertices(positions);
    VAO.setLayout({3}, false);
    VAO.setLayout({4}, true);
    // define cube scale
    model_matrix = glm::scale(glm::vec3(1));
  }
  void draw();
  void update_instances();
  void create_sphere(float Radius);
  void create_sprite(float Radius);

  // fluid sim helper stems
  int hash(glm::vec3);
  float poly6(float r);
  glm::vec3 poly6_grad(glm::vec3 r);
  glm::vec3 spiky_grad(glm::vec3 r);
  float laplacian_visc(glm::vec3 r);
  float C(float r);
  // fluid simulation steps
  void step_physics(int n);
  void find_neighbors();
  void compute_pressure();
  void compute_forces();
  void compute_position();
};

#endif