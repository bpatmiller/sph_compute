#version 430 core

layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(std430) struct Particle
{
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

layout(std430, binding=0) buffer ParticleBlock
{
    Particle particles[];
};

const float PI =  3.1415927410125732421875f;

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

vec3 spiky_grad(vec3 r) {

}

void main() {
      uint i = gl_WorkGroupID.x;
        vec3 force = vec3(0);
      for (uint j = 0; j < particles_size; j++) {
        float r = distance(particles[i].position, particles[j].position);
        if (i == j || r == 0)
            continue;
        if (r < h ) {
            // pressure force
            float pres_coef = particles[i].density * MASS * (particles[i].pressure + particles[j].pressure) / (2.0 * particles[j].density);
            force += spiky_grad(particles[i].position - particles[j].position) * pres_coef;
            // float viscosity force
            force += VISC * MASS *((particles[j].velocity - particles[i].velocity) / particles[])
            // TODO surface tension force

        }        
      }
 
}