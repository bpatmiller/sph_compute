#version 330 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 offset_position;

uniform mat4 model;

void main() {
  gl_Position = vec4(offset_position, 0) + (model * vec4(vertex_position, 1.0));
}
