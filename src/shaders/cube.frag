#version 330 core
out vec4 fragment_color;
uniform vec3 light_position;

in vec3 light_direction;
in vec3 view_direction;
in vec3 normal;
in vec3 triangle_coords;
in float scale;
in float density;

void main() {
  // float wireframe_min =
  //     scale * min(triangle_coords.x, min(triangle_coords.y,
  //     triangle_coords.z));

  // if (wireframe_min < 0.001) {
  //   fragment_color = mix(vec4(1, 1, 1, 1), vec4(0,0,0,1), density/50);

  // } else {
  fragment_color = mix(vec4(0.2, 0.4, 0.65, 1), vec4(0,0,0,1), density/25);
  // }
}
