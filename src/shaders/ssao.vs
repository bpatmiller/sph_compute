#version 430 core
in vec3 vertex_position;

out vec2 uv;

void main() {
  uv = (vertex_position.xy + vec2(1, 1)) / 2.0;
  gl_Position = vec4(vertex_position, 1.0);
}
