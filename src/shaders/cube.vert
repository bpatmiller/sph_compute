#version 330 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 offset_position;

uniform mat4 model;
out float d;

void main() {
  d = offset_position[3];
  gl_Position =
      vec4(offset_position.xyz, 0) + (model * vec4(vertex_position, 1.0));
}
