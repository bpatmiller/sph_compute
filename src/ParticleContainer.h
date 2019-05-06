#ifndef PARTICLECONTAINER_H
#define PARTICLECONTAINER_H

#include "Particle.h"
#include "VAO.h"
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>

struct ParticleContainer {
  VertexArr VAO;
  glm::mat4 model_matrix;
  std::vector<glm::vec3> vertices;
  std::vector<glm::uvec3> face_indices;
  std::vector<glm::vec4> positions;

  std::vector<Particle> particles;
  std::unordered_multimap<int, Particle *> block_hashmap;

  // physics parameters
  float grid_cell_size;
  float smoothing_radius;
  int grid_n = 0;
  float timestep = 0.001;

  float mass = 28.0;
  float rest_density = 59.0;
  float gas_constant = 0.000001;
  float viscosity = 1.3;
  float surface_tension_coef = 8.0;
  float stiffness = 7.0;
  float damping = 0.1;

  ParticleContainer(glm::vec3 min, glm::vec3 max, float radius) {
    grid_cell_size = radius * 4;
    smoothing_radius = grid_cell_size;

    // number of grid subdivisions
    grid_n = glm::pow((max.x - min.x) / (radius * 2), 3);

    // load sphere mesh
    create_sphere(radius);
    // generate initial positions
    for (float x = min.x; x <= max.x; x += radius * 2) {
      for (float y = min.y; y <= max.y; y += radius * 2) {
        for (float z = min.z; z <= max.z; z += radius * 2) {
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

  // fluid sim helper stems
  int get_cell_hash(glm::vec3);
  float smoothing_kernel(float dist);
  glm::vec3 smoothing_kernel_gradient(glm::vec3 r);
  float smoothing_kernel_gradient_squared(glm::vec3 r);
  // fluid simulation steps
  void step_physics(int n);
  void find_neighboors();
  void compute_density();
  void compute_normals();
  void compute_pressure();
  void compute_forces();
  void compute_velocity();
  void compute_position();
  void resolve_collision();
};

#endif