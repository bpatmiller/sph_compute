#version 330 core
in vec3 vertex_position;

void main() { gl_Position = vec4(vertex_position, 1.0); }
