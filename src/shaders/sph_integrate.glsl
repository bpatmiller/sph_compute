#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// layout(std430)
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

layout(std430, binding = 1) buffer HashToIndexBlock { int HashToIndex[]; };

const float PI = 3.1415927410125732421875f;

uniform int num_cells;
uniform float h;
uniform float MASS;
uniform float timestep;
uniform vec3 box_dimensions;
uniform vec3 focus;
uniform bool pipe;

float EPS = h * 0.025f;
float damping = -0.5;
uint hash(vec3 position) {
  vec3 p_hat = floor(position / h);
  return ((uint(p_hat.x) * 73856093) ^ (uint(p_hat.y) * 19349663) ^
          (uint(p_hat.z) * 83492791)) %
         num_cells;
}

void main() {
  uint i = gl_WorkGroupID.x;
  Particle p = particles[i];

  // verlet integration
  vec3 new_pos = p.position + p.velocity * timestep +
                 (p.acceleration * timestep * timestep * 0.5f);
  vec3 new_acc = p.force / MASS;
  vec3 new_vel = p.velocity + (p.acceleration + new_acc) * (0.5 * timestep);
  p.position = new_pos;
  p.velocity = new_vel;
  p.acceleration = new_acc;

  // check if put into inlet pipe
  vec3 p_pos = particles[i].position;
  if (pipe) {
    if (p_pos.y < 3.0 * EPS &&
        pow(p_pos.x - focus.x, 2) + pow(p_pos.z - focus.z, 2) < 0.025) {

      float offset_y = p_pos.x - focus.x;
      float offset_z = p_pos.z - focus.z;

      p.position.x = p_pos.y;
      p.position.y = 3.0 * focus.y + offset_y;
      p.position.z = focus.z + offset_z;

      float vx = 2.0 - p.velocity.y;
      p.velocity.y = p.velocity.x * 0.1;
      p.velocity.x = vx;
      p.velocity.z *= 0.1;

      // break early
      p._pad0 = HashToIndex[int(p.hash)];
      p.hash = hash(p.position);
      particles[i] = p;
      return;
    }
  }
  // check collisions
  for (uint var = 0; var < 3; var++) {
    if (p_pos[var] < 0) {
      p.position[var] = EPS;
      p.velocity[var] *= damping;

    } else if (p_pos[var] > box_dimensions[var]) {
      p.position[var] = box_dimensions[var] - EPS;
      p.velocity[var] *= damping;
    }
  }

  p.hash = hash(p.position);
  p._pad0 = HashToIndex[int(p.hash)];
  particles[i] = p;
}