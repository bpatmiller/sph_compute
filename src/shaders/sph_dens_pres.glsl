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

uniform int particles_size;
uniform int num_cells;
uniform float time;
uniform float h;
uniform float MASS;
uniform float GAS_CONST;
uniform float REST_DENS;

float poly6(float r) {
  return 315.0f * pow(h * h - r * r, 3.0f) / (64.0f * PI * pow(h, 9.0f));
}

int hash(vec3 position) {
  vec3 p_hat = floor(position / h);
  return ((int(p_hat.x) * 73856093) ^ (int(p_hat.y) * 19349663) ^
          (int(p_hat.z) * 83492791)) %
         num_cells;
}

void main() {
  uint i = gl_WorkGroupID.x;
  float dens = 0;
  Particle p = particles[i];

  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      for (int z = -1; z <= 1; z++) {
        vec3 current_pos = p.position + vec3(x * h, y * h, z * h);
        int current_hash = hash(current_pos);
        int start_index = HashToIndex[current_hash];
        for (uint j = start_index; hash(particles[j].position) == current_hash;
             j++) {
          float r = distance(p.position, particles[j].position);
          if (r < h) {
            dens += MASS * poly6(r);
          }
        }
      }
    }
  }

  particles[i].density = dens;
  // particles[i].pressure = GAS_CONST * (dens - REST_DENS);
  particles[i].pressure = GAS_CONST * ( pow(dens / REST_DENS, 7) -1 );

}