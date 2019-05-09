#version 430 core

layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(std430) struct Particle
{
  vec3 position;
  float density; // 4
  vec3 velocity;
  float pressure; // 8
  vec3 acceleration;
  float color; // 12
  vec3 force;
  float _pad0; // 16
  vec3 normal;
  float _pad1; // 20
};

layout(std430, binding=0) buffer ParticleBlock
{
    Particle particle[];
};

uniform float time;

void main() {
    // give each compute invocation a unique ID.
    // // it is assumed that the dimensions of gl_NumWorkGroups match the size of the cloth
    // uint x_index = gl_WorkGroupID.x;
    // uint y_index = gl_WorkGroupID.y;
    // uint z_index = gl_WorkGroupID.z;
    // uint index = gl_NumWorkGroups.x * y_index + x_index;
    uint index = gl_WorkGroupID.x;
    particle[index].position += vec3(0, 0.01 * sin(time), 0);
    // vec3 temp = vertex[index].position;
    // vertex[index].position = 2.0 * vertex[index].position - vertex[index].prev_pos + vec3(0.0,-9.8,0.0) * time_step * time_step;
    // vertex[index].prev_pos = temp;

  
}