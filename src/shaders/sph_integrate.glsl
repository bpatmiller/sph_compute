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

const float PI = 3.1415927410125732421875f;
const float EPS = 0.0001;

uniform int particles_size;
uniform int num_cells;
uniform float time;
uniform float h;
uniform float MASS;
uniform float GAS_CONST;
uniform float REST_DENS;
uniform float VISC;
uniform float timestep;
uniform vec3 box_dimensions;

uint hash(vec3 position) {
  vec3 p_hat = floor(position / h);
  return ((uint(p_hat.x) * 73856093) ^ (uint(p_hat.y) * 19349663) ^
          (uint(p_hat.z) * 83492791)) %
         num_cells;
}

void main() {
  uint i = gl_WorkGroupID.x;
  Particle p = particles[i];

  vec3 new_accel = p.force / MASS;
  // get new velocity
  p.velocity += timestep * ((new_accel + p.acceleration) * 0.5f);
  // get new position
  p.position +=
      (timestep * p.velocity) + (p.acceleration * timestep * timestep * 0.5f);
  // update accel
  p.acceleration = new_accel;

  // check collisions - FIXME abstract this now, for now
  // keep in a 2x2x2 bounding box
  vec3 p_pos = particles[i].position;
  for (uint var = 0; var < 3; var++) {
    if (p_pos[var] < 0) {
      p.position[var] = EPS;
      p.velocity[var] *= -0.5;

    } else if (p_pos[var] > box_dimensions[var]) {
      p.position[var] = box_dimensions[var] - EPS;
      p.velocity[var] *= -0.5;
    }
  }

  p.hash = hash(p.position);
  particles[i] = p;
}