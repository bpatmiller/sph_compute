#include "ParticleContainer.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <math.h>
#include <omp.h>

// ================================================
// rendering functions
// ================================================
void ParticleContainer::create_sprite(float Radius) {
  vertices = {
      {-Radius, Radius, 0.0f},
      {-Radius, -Radius, 0.0f},
      {Radius, Radius, 0.0f},
      {Radius, -Radius, 0.0f},
  };
}
void ParticleContainer::draw() {
  VAO.bind();
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, positions.size());
}
void ParticleContainer::update_instances() {
  positions.clear();
  for (auto p : particles) {
    positions.emplace_back(glm::vec4(p.position, glm::length(p.velocity)));
  }
  VAO.ib.bindVertices(positions);
}

// ================================================
// smoothing kernel functions
// ================================================
float ParticleContainer::poly6(float r) {
  return poly6_coef * glm::pow(glm::pow(h, 2) - glm::pow(r, 2), 3);
}

glm::vec3 ParticleContainer::poly6_grad(glm::vec3 r) {
  return poly6_grad_coef * r *
         glm::pow((glm::pow(h, 2.0f) - glm::pow(glm::length(r), 2.0f)), 2.0f);
}

glm::vec3 ParticleContainer::spiky_grad(glm::vec3 r) {
  return spiky_grad_coef * glm::normalize(r) *
         glm::pow(h - glm::length(r), 2.0f);
}

float ParticleContainer::laplacian_visc(glm::vec3 r) {
  return laplacian_visc_coef * (h - glm::length(r));
}

float ParticleContainer::C(float r) {
  if (2 * r > h && r <= h) {
    return C_coef * glm::pow(h - r, 3) * glm::pow(r, 3);
  } else if (r > 0 && 2 * r <= h) {
    return (C_coef * 2 * glm::pow(h - r, 3) * glm::pow(r, 3)) -
           (glm::pow(h, 6) / 64);
  }
  return 0;
}

// ================================================
// spatial hashing
// ================================================

int ParticleContainer::hash(glm::vec3 p) {
  glm::ivec3 p_hat(glm::floor(p / h));
  return ((p_hat.x * 73856093) ^ (p_hat.y * 19349663) ^ (p_hat.z * 83492791));
  // TODO potentially mod by grid size
}

// ================================================
// simulation steps
// ================================================

// 1) find each particles neighbors
//  and compute density at each particle
void ParticleContainer::find_neighbors() {
  // build unordered multimap
  block_hashmap.clear();
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    block_hashmap.insert({hash(p.position), &p});
  }

// iterate through multimap and create neighbors
#pragma omp parallel for
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    p.neighbors.clear();
    // for all 27 adjacent spatial blocks
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        for (int k = -1; k <= 1; k++) {
          glm::vec3 cur_pos = p.position + glm::vec3(h * i, h * j, h * k);
          int block_hash = hash(cur_pos);
          // iterate through candidates
          auto iter = block_hashmap.equal_range(block_hash);
          for (auto it = iter.first; it != iter.second; it++) {
            float dist = glm::distance(p.position, it->second->position);
            if (0 < dist && dist < h) {
              p.neighbors.emplace_back(it->second);
              // compute density while doing this
              p.density +=
                  mass * poly6(glm::distance(p.position, it->second->position));
            }
          }
        }
      }
    }
  }
}

// 2) compute pressure and normals for each particle
void ParticleContainer::compute_pressure() {
#pragma omp parallel for
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    // compute pressure
    p.pressure = gas_constant * (p.density - rest_density);
    // compute normals
    p.normal = glm::vec3(0);
    for (auto n : p.neighbors) {
      p.normal += (mass / n->density) * poly6_grad(p.position - n->position);
    }
    p.normal *= h;
  }
}

// 3) compute the forces acting on each particle
// gravity, pressure, viscosity, surface tension
void ParticleContainer::compute_forces() {
#pragma omp parallel for
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    // f_gravity
    p.force = glm::vec3(0, -9.8, 0);
    // f_pressure
    for (auto n : p.neighbors) {
      float coef = -mass * (p.pressure + n->pressure) / (2 * n->density);
      p.force += poly6_grad(p.position - n->position) * coef;
    }
    // f_viscosity
    for (auto n : p.neighbors) {
      p.force += viscosity * mass * ((p.velocity - n->velocity) / n->density) *
                 laplacian_visc(p.position - n->position);
    }
    // f_surface
    for (auto n : p.neighbors) {
      float k = 2 * rest_density / (p.density + n->density);
      glm::vec3 cohesion = -surface_tension_coef * mass * mass *
                           C(glm::length(p.position - n->position)) *
                           glm::normalize(p.position - n->position);
      glm::vec3 curvature =
          -surface_tension_coef * mass * (p.normal - n->normal);
      p.force += k * (cohesion + curvature);
    }
    // clamp for reasonable measure and add gravity
  }
}

// 4) compute new
void ParticleContainer::compute_position() {
#pragma omp parallel for
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    // get current acceleration
    p.acceleration_old = p.acceleration;
    p.acceleration = (p.force / mass);
    // get new velocity
    p.velocity += timestep * ((p.acceleration + p.acceleration_old) * 0.5f);

    // get new position
    p.position +=
        (timestep * p.velocity) + (p.acceleration * timestep * timestep * 0.5f);
    // handle collisions
    if (p.position.y < container_min.y) {
      if (false && glm::length(glm::vec2(p.position.x, p.position.z)) < 0.2) {
        p.position.y = container_min.y;
        p.velocity.y = 1.0;
      } else {
        p.position.y = container_min.y;
        p.velocity.y = -p.velocity.y * damping;
      }
    } else if (p.position.y > container_max.y) {
      p.position.y = container_max.y;
      p.velocity.y = -p.velocity.y * damping;
    }
    if (p.position.x > container_max.x) {
      p.position.x = container_max.x;
      p.velocity.x = -p.velocity.x * damping;
    } else if (p.position.x < container_min.x) {
      p.position.x = container_min.x;
      p.velocity.x = -p.velocity.x * damping;
    }
    if (p.position.z > container_max.z) {
      p.position.z = container_max.z;
      p.velocity.z = -p.velocity.z * damping;
    } else if (p.position.z < container_min.z) {
      p.position.z = container_min.z;
      p.velocity.z = -p.velocity.z * damping;
    }
  }
}

void ParticleContainer::step_physics(int n) {
  for (int i = 0; i < n; i++) {
    find_neighbors();
    compute_pressure();
    compute_forces();
    compute_position();
  }
}