#version 330 core
out vec4 fragment_color;
uniform vec3 light_position;

// in vec3 light_direction;
// in vec3 view_direction;
// in vec3 normal;
// in vec3 triangle_coords;
// in float scale;
in float density;

void main() {
  fragment_color = mix(vec4(0,0,0,1), vec4(0.2, 0.4, 0.65, 1), density + 0.1);
}
