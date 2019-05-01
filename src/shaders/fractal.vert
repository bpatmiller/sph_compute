#version 330 core
in vec3 vertex_position;

out vec2 uv;

void main() {
  uv = vertex_position.xy;
  gl_Position = vec4(vertex_position, 1.0);
}
