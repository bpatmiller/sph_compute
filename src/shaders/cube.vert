#version 330 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 offset_position;

uniform mat4 projection;
uniform mat4 view;
out float d;

void main() {
  d = offset_position.w;
  gl_Position =
      projection * view * vec4(vertex_position.xyz + offset_position.xyz, 1.0);
}
