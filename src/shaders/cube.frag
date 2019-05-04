#version 330 core
out vec4 fragment_color;
uniform vec3 light_position;

in vec3 light_direction;
in vec3 view_direction;
in vec3 normal;
in vec3 triangle_coords;
in float scale;

void main() {
  vec3 lv = light_direction + view_direction;
  vec3 h = lv / length(lv);
  float bp_shade = dot(normal, h);

  vec3 df = normal; // vec3(1,0,0);
  vec3 am = vec3(1, 1, 1);
  vec3 color = 0.1 * am + bp_shade * df;
  fragment_color = vec4(color, 1);

  float wireframe_min =
      scale * min(triangle_coords.x, min(triangle_coords.y, triangle_coords.z));

  if (wireframe_min < 0.01)
    fragment_color = vec4(1, 0, 0, 1);
}
