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

uniform int particles_size;
uniform int num_cells;
uniform float time;
uniform float h;
uniform float MASS;
uniform float GAS_CONST;
uniform float REST_DENS;
uniform float VISC;
uniform float timestep;

void main() {
  uint i = gl_WorkGroupID.x;

  vec3 new_accel = particles[i].force / MASS;
  // get new velocity
  particles[i].velocity +=
      timestep * ((new_accel + particles[i].acceleration) * 0.5f);
  // get new position
  particles[i].position +=
      (timestep * particles[i].velocity) +
      (particles[i].acceleration * timestep * timestep * 0.5f);
  // update accel
  particles[i].acceleration = new_accel;

  // check collisions - FIXME abstract this now, for now
  // keep in a 2x2x2 bounding box
  vec3 p_pos = particles[i].position;
  for (uint var = 0; var < 3; var++) {
    if (p_pos[var] < -0.01) {
      particles[i].position[var] = 0;
      particles[i].velocity[var] *= -1;

    } else if (p_pos[var] > 2.01) {
      particles[i].position[var] = 2;
      particles[i].velocity[var] *= -1;
    }
  }
}