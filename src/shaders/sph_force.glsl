#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430) struct Particle {
  vec3 position;
  float density; // 4
  vec3 velocity;
  float pressure; // 8
  vec3 acceleration;
  float hash; // 12
  vec3 force;
  float _pad0; // 16
  vec3 normal;
  float _pad1; // 20
};

layout(std430, binding = 0) buffer ParticleBlock { Particle particles[]; };

layout(std430, binding = 1) buffer HashToIndexBlock { int HashToIndex[]; };

const float PI = 3.1415927410125732421875f;

uniform int particles_size;
uniform int num_cells;
uniform float time;
uniform float h;
uniform float MASS;
uniform float GAS_CONST;
uniform float REST_DENS;
uniform float VISC;

vec3 spiky_grad(vec3 r, float l) {
  return normalize(r) * pow(h - length(r), 2.0) * (45.0 / (PI * pow(h, 6)));
}

float visc_lapl(vec3 r) {
  return (45.0f / (PI * pow(h, 6.0))) * (h - length(r));
}

int hash(vec3 position) {
  vec3 p_hat = floor(position / h);
  return ((int(p_hat.x) * 73856093) ^ (int(p_hat.y) * 19349663) ^
          (int(p_hat.z) * 83492791)) %
         num_cells;
}

void main() {
  uint i = gl_WorkGroupID.x;
  vec3 force = MASS * vec3(0, -9.8, 0);

  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      for (int z = -1; z <= 1; z++) {
        vec3 current_pos = particles[i].position + vec3(x * h, y * h, z * h);
        int current_hash = hash(current_pos);
        int start_index = HashToIndex[current_hash];
        for (uint j = start_index; hash(particles[j].position) == current_hash;
             j++) {
          if (i == j)
            continue;
          float r = distance(particles[i].position, particles[j].position);
          if (r < h && 0 < r) {
            // compute forces
            // pressure
            float pres_coef = particles[i].density * MASS *
                              (particles[i].pressure + particles[j].pressure) /
                              (2.0 * particles[j].density);
            force +=
                spiky_grad(particles[i].position - particles[j].position, r) *
                pres_coef;
            // viscosity
            force += VISC * MASS *
                     ((particles[j].velocity - particles[i].velocity) /
                      particles[j].density) *
                     visc_lapl(particles[i].position - particles[j].position);
            // TODO surface tension
            //             float k = 2 * REST_DENSITY / (p.density +
            //             n->density);
            // glm::vec3 cohesion = -SURF * MASS *
            //                      C(glm::length(p.position - n->position)) *
            //                      glm::normalize(p.position - n->position);
            // glm::vec3 curvature = -SURF * MASS * (p.normal - n->normal);
            // glm::vec3 f_surf = k * (cohesion + curvature);

            // p.force += f_pres + f_visc + f_surf;
          }
        }
      }
    }
  }

  particles[i].force = force;

  // brute force approach
  // for (uint j = 0; j < particles_size; j++) {
  //   float r = distance(particles[i].position, particles[j].position);
  //   if (i == j || r == 0)
  //     continue;
  //   if (r < h) {
  //     // pressure force
  //     float pres_coef = particles[i].density * MASS *
  //                       (particles[i].pressure + particles[j].pressure) /
  //                       (2.0 * particles[j].density);
  //     force +=
  //         spiky_grad(particles[i].position - particles[j].position) *
  //         pres_coef;
  //     // float viscosity force
  //     force += VISC * MASS *
  //              ((particles[j].velocity - particles[i].velocity) /
  //              particles[])
  //     // TODO surface tension force
  //   }
  // }
}