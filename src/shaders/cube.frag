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
  vec3 orange = vec3(0.8, 0.4, 0.2);
  vec3 blue = vec3(0.3, 0.45, 0.7);

  vec3 lv = light_direction + view_direction;
  vec3 h = lv / length(lv);
  float bp_shade = dot(normal, h);

  vec3 am = vec3(1, 1, 1);
  vec3 color = 0.1 * am + bp_shade * mix(blue, orange, density / 100);
  fragment_color = vec4(color, 1);

  // float wireframe_min =
  //     scale * min(triangle_coords.x, min(triangle_coords.y,
  //     triangle_coords.z));

  // if (wireframe_min < 0.001)
  //   fragment_color = vec4(1, 1, 1, 1);
}
