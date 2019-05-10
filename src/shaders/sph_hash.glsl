#version 430 core

layout(local_size_x = 250, local_size_y = 1, local_size_z = 1) in;

struct Particle {
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

layout(std430, binding = 1) buffer AccelerationBlock {
  uint hash_to_particles[];
};

const float PI = 3.1415927410125732421875f;

uniform int particles_size;
uniform int num_cells;
uniform float h;

uint hash(vec3 position) {
  vec3 p_hat = floor(position / h);
  return ((int(p_hat.x) * 73856093) ^ (int(p_hat.y) * 19349663) ^
          (int(p_hat.z) * 83492791)) %
         num_cells;
}

uint hash_to_particle_index(uint i) { return i * (particles_size / 8); }

void main() {
  uint i = gl_LocalInvocationID.x;
  uint loops = particles_size / 250;

  for (uint j = 0; j < 250; j++) {
    particles[i * loops + j].hash = hash(particles[i * loops + j].position);
  }
}