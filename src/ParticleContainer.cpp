#include "ParticleContainer.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <math.h>
#include <omp.h>

// ================================================
// rendering functions
// ================================================
void ParticleContainer::create_sphere(float Radius) {
  int Stacks = 5;
  int Slices = 5;

  for (int i = 0; i <= Stacks; ++i) {
    float V = i / (float)Stacks;
    float phi = V * glm::pi<float>();
    for (int j = 0; j <= Slices; ++j) {
      float U = j / (float)Slices;
      float theta = U * (glm::pi<float>() * 2);
      float x = cosf(theta) * sinf(phi);
      float y = cosf(phi);
      float z = sinf(theta) * sinf(phi);
      vertices.push_back(glm::vec3(x, y, z) * Radius);
    }
  }

  for (int i = 0; i < Slices * Stacks + Slices; ++i) {
    face_indices.emplace_back(glm::uvec3(i, i + Slices + 1, i + Slices));
    face_indices.emplace_back(glm::uvec3(i + Slices + 1, i, i + 1));
  }
}

void ParticleContainer::draw() {
  VAO.bind();
  glDrawElementsInstanced(GL_TRIANGLES, face_indices.size() * 3,
                          GL_UNSIGNED_INT, face_indices.data(),
                          positions.size());
}

void ParticleContainer::update_instances() {
  positions.clear();
  for (auto p : particles) {
    positions.emplace_back(glm::vec4(p.position, p.color));
  }
  VAO.ib.bindVertices(positions);
}

// ================================================
// smoothing kernel functions
// ================================================
float ParticleContainer::poly6(float r) {
  return POLY6_COEF * glm::pow(glm::pow(h, 2) - glm::pow(r, 2), 3);
}

glm::vec3 ParticleContainer::poly6_grad(glm::vec3 r) {
  return POLY6_GRAD_COEF * r *
         glm::pow((glm::pow(h, 2.0f) - glm::pow(glm::length(r), 2.0f)), 2.0f);
}

float ParticleContainer::spiky(float r) {
  return SPIKY_COEF * glm::pow(h - r, 3.0f);
}

glm::vec3 ParticleContainer::spiky_grad(glm::vec3 r) {
  return spiky_grad_coef * glm::normalize(r) *
         glm::pow(h - glm::length(r), 2.0f);
}

float ParticleContainer::poly6_laplac(glm::vec3 r) {
  return POLY6_GRAD_COEF * (glm::pow(h, 2.0f) - glm::length2(r)) *
         (3.0f * h * h - 7 * glm::length2(r));
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
  return ((p_hat.x * 73856093) ^ (p_hat.y * 19349663) ^ (p_hat.z * 83492791)) %
         NUM_CELLS;
}

// ================================================
// simulation steps
// ================================================

// 1) find each particles neighbors
//  and compute density at each particle
void ParticleContainer::compute_neighbors_density() {
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
    p.density = 0;
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
            if (dist < h) {
              p.neighbors.emplace_back(it->second);
              // compute density while doing this
              p.density +=
                  MASS * poly6(glm::distance(p.position, it->second->position));
            }
          }
        }
      }
    }
  }
}

// 2) compute pressure and normans = 1ls for each particle
void ParticleContainer::compute_pressure() {
#pragma omp parallel for
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    // compute pressure
    p.pressure = GAS_CONST * (p.density - REST_DENSITY);
    // compute normals
    p.normal = glm::vec3(0);
    for (auto n : p.neighbors) {
      if (&p == n)
        continue;
      p.normal += (MASS / n->density) * poly6_grad(p.position - n->position);
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
    p.force = MASS * glm::vec3(0, -9.8, 0);
    // f_pressure
    for (auto n : p.neighbors) {
      if (&p == n || p.position - n->position == glm::vec3(0))
        continue;

      // f_pres
      float coef =
          p.density * MASS * (p.pressure + n->pressure) / (2.0f * n->density);
      glm::vec3 f_pres = spiky_grad(p.position - n->position) * coef;

      // f_viscosity
      glm::vec3 f_visc = VISC * MASS *
                         ((n->velocity - p.velocity) / n->density) *
                         laplacian_visc(p.position - n->position);

      // f_surface
      float k = 2 * REST_DENSITY / (p.density + n->density);
      glm::vec3 cohesion = -SURF * MASS *
                           C(glm::length(p.position - n->position)) *
                           glm::normalize(p.position - n->position);
      glm::vec3 curvature = -SURF * MASS * (p.normal - n->normal);
      glm::vec3 f_surf = k * (cohesion + curvature);

      p.force += f_pres + f_visc + f_surf;
    }
  }
}

// 4) compute new
void ParticleContainer::integrate() {
#pragma omp parallel for
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    // get current acceleration
    p.acceleration_old = p.acceleration;
    p.acceleration = (p.force / MASS);
    // get new velocity
    // FIXME get averaged velocities
    p.velocity += timestep * ((p.acceleration + p.acceleration_old) * 0.5f);
    // get new position
    p.position +=
        (timestep * p.velocity) + (p.acceleration * timestep * timestep * 0.5f);

    // fountain!
    if (fountain && p.position.y < container_min.y + 0.25 &&
        p.position.x < container_min.x + 0.1 && p.position.z < 0.25 &&
        p.position.z > -0.25) {
      p.position.y = container_min.y + 0.25;
      p.position.x = container_min.x + 0.1;
      p.velocity.y *= damping;
      p.velocity.x *= damping;
      p.velocity.y += 7.0;
      p.velocity.x += 1.5;
    }

    // other fountain
    if (fountain && p.position.y < container_min.y + 0.25 &&
        p.position.x > container_max.x - 0.1 && p.position.z < 0.25 &&
        p.position.z > -0.25) {
      p.position.y = container_min.y + 0.25;
      p.position.x = container_max.x - 0.1;
      p.velocity.y *= damping;
      p.velocity.x *= damping;
      p.velocity.y += 7.0;
      p.velocity.x -= 1.5;
    }

    // handle collisions
    for (uint j = 0; j < 3; j++) {
      if (p.position[j] < container_min[j] - 0.001) {
        p.position[j] = container_min[j];
        p.velocity[j] *= damping;
      } else if (p.position[j] > container_max[j] + 0.001) {
        p.position[j] = container_max[j];
        p.velocity[j] *= damping;
      }
    }
    if (glm::isnan(p.position.x) || glm::isnan(p.position.y) ||
        glm::isnan(p.position.z)) {
      std::cerr << "!! ERROR particle position isNaN" << std::endl;
    }
  }
}

void ParticleContainer::step_physics(int n) {
  for (int i = 0; i < n; i++) {
    compute_neighbors_density();
    compute_pressure();
    compute_forces();
    integrate();
  }
}