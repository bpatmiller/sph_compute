#version 330 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 position;
layout(location = 2) in float density;
layout(location = 3) in vec3 velocity;
layout(location = 4) in float pressure;
layout(location = 5) in vec3 acceleration;
layout(location = 6) in float color;
layout(location = 7) in vec3 force;
layout(location = 8) in float pad0;
layout(location = 9) in vec3 normal;
layout(location = 10) in float pad1;

uniform mat4 projection;
uniform mat4 view;

void main() {
  gl_Position = projection * view * vec4(position + vertex_position, 1.0);
}
