#version 430 core
out vec4 fragment_color;

in vec3 world_position;

uniform vec3 camera_position;

void main() {
  // float h = (normalize(world_position - camera_position).y * 2) + 0.5;
  // if (h < 0) {
  //   h = h * -5.0;
  // }
  // fragment_color =
  //     vec4(mix(vec3(0.53, 0.1, 0.8), vec3(0, 0, 0.5), clamp(h, 0.0, 1.0)),
  //     0.9);
  fragment_color =
      vec4(vec3(0.25) + normalize(abs(world_position - camera_position)) * 0.25,
           0.9);
}
