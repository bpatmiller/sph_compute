#version 330 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 offset_position;

out float d;
uniform mat4 inverse_rotation;

void main() {
  d = offset_position[3];
  gl_Position = vec4(offset_position.xyz, 0) +
                (inverse_rotation * vec4(vertex_position.xyz, 1.0));
}
