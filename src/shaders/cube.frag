#version 430 core
out vec4 fragment_color;

in vec3 v_color;

void main() { fragment_color = vec4(v_color, 1.0); }