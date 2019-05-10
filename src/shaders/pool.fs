#version 430 core
out vec4 fragment_color;
// in vec3 world_position;

void main() {
  // fragment_color =
  //     vec4(world_position.x, world_position.y, world_position.z, 1);
  fragment_color = vec4(0.5, 0.5, 0.5, 1.0);
}
