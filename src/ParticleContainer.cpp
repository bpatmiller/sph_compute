#include "ParticleContainer.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <math.h>

void ParticleContainer::create_sphere(float Radius) {
  int Stacks = 8;
  int Slices = 8;

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
    positions.emplace_back(glm::vec4(p.position, glm::length(p.force)));
  }
  VAO.ib.bindVertices(positions);
}

int ParticleContainer::get_cell_hash(glm::vec3 p) {
  glm::ivec3 p_hat(floor(p.x / grid_cell_size), floor(p.y / grid_cell_size),
                   floor(p.z / grid_cell_size));
  return ((p_hat.x * 73856093) ^ (p_hat.y * 19349663) ^ (p_hat.z * 83492791)) %
         grid_n;
}

void ParticleContainer::find_neighboors() {
  //   build unordered multimap
  block_hashmap.clear();
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    block_hashmap.insert({get_cell_hash(p.position), &p});
  }

  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    p.neighbors.clear();

    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        for (int k = -1; k <= 1; k++) {
          glm::vec3 block_position =
              p.position + glm::fvec3(i, j, k) * grid_cell_size;
          int block_hash = get_cell_hash(block_position);
          // get the neighbors that have the same hash
          auto iter = block_hashmap.equal_range(block_hash);
          for (auto it = iter.first; it != iter.second; it++) {
            float dist = glm::distance(p.position, it->second->position);
            if (0 < dist && dist < smoothing_radius) {
              p.neighbors.emplace_back(it->second);
            }
          }
        }
      }
    }
  }
}

float ParticleContainer::smoothing_kernel(float dist) {
  return (315 / (64 * 3.141592 * glm::pow(smoothing_radius, 9))) *
         glm::pow(glm::pow(smoothing_radius, 2) - glm::pow(dist, 2), 3);
}

glm::vec3 ParticleContainer::smoothing_kernel_gradient(glm::vec3 r) {
  float coef =
      (945 / (32 * 3.141592 * glm::pow(smoothing_radius, 9))) *
      glm::pow(glm::pow(smoothing_radius, 2) - glm::pow(glm::length(r), 2), 2);
  return r * coef;
}

float ParticleContainer::smoothing_kernel_gradient_squared(glm::vec3 r) {
  return (45 / (3.141592 * glm::pow(smoothing_radius, 6))) *
         (smoothing_radius - glm::length(r));
}

void ParticleContainer::compute_density() {
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    p.density = 0;
    for (auto neighbor : p.neighbors) {
      p.density += mass * smoothing_kernel(
                              glm::distance(p.position, neighbor->position));
    }
  }
}

float ParticleContainer::C(float r) {
  if (2 * r > smoothing_radius && r <= smoothing_radius) {
    float coef = (32 / (3.141592 * glm::pow(smoothing_radius, 9)));
    return coef * glm::pow(smoothing_radius - r, 3) * glm::pow(r, 3);
  } else if (r > 0 && 2 * r <= smoothing_radius) {
    float coef = (32 / (3.141592 * glm::pow(smoothing_radius, 9)));
    return coef * 2 * glm::pow(smoothing_radius - r, 3) * glm::pow(r, 3) -
           glm::pow(smoothing_radius, 6) / 64;
  }
  return 0;
}

void ParticleContainer::compute_pressure() {
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    // compute pressure
    p.pressure = gas_constant * (p.density - rest_density);
    // compute normals
    p.normal = glm::vec3(0);
    for (auto n : p.neighbors) {
      p.normal += (mass / n->density) *
                  smoothing_kernel_gradient(p.position - n->position);
    }
    p.normal *= smoothing_radius;
  }
}
void ParticleContainer::compute_forces() {
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    p.force = glm::vec3(0);
    // f_pressure
    for (auto n : p.neighbors) {
      float coef = -mass * ((p.pressure / glm::pow(p.density, 2)) +
                            (n->pressure / glm::pow(n->density, 2)));
      p.force += smoothing_kernel_gradient(p.position - n->position) * coef;
    }
    // f_viscosity
    for (auto n : p.neighbors) {
      p.force += viscosity * mass * ((p.velocity - n->velocity) / n->density) *
                 smoothing_kernel_gradient_squared(p.position - n->position);
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
    // manually tune params
    p.force /= 50;
    // clamp for reasonable measure and add gravity
    p.force = glm::clamp(p.force, -100.0f, 100.0f);
    p.force += glm::vec3(0, -9.8, 0);
  }
}
void ParticleContainer::compute_position() {
  for (uint i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    // get new velocity
    p.velocity += timestep * (p.force / mass);
    // get new position
    p.position += timestep * p.velocity;
    // handle collisions
    if (p.position.y < -container_depth) {
      if (glm::length(glm::vec2(p.position.x, p.position.z)) < 0.1) {
        p.position.y = -container_depth;
        p.velocity.y = 1.0;
      } else {
        p.position.y = -container_depth;
        p.velocity.y = -p.velocity.y * damping;
      }
    } else if (p.position.y > container_depth) {
      p.position.y = container_depth;
      p.velocity.y = -p.velocity.y * damping;
    }
    if (p.position.x > container_width) {
      p.position.x = container_width;
      p.velocity.x = -p.velocity.x * damping;
    } else if (p.position.x < -container_width) {
      p.position.x = -container_width;
      p.velocity.x = -p.velocity.x * damping;
    }
    if (p.position.z > container_width) {
      p.position.z = container_width;
      p.velocity.z = -p.velocity.z * damping;
    } else if (p.position.z < -container_width) {
      p.position.z = -container_width;
      p.velocity.z = -p.velocity.z * damping;
    }
  }
}

void ParticleContainer::step_physics(int n) {
  for (int i = 0; i < n; i++) {
    find_neighboors();
    compute_density();
    compute_pressure();
    compute_forces();

    compute_position();
  }
}