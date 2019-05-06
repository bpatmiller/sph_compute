#version 330 core
out vec4 fragment_color;

in vec3 world_position;

void main() {
   if (mod(floor(10 * world_position.x)+floor(10 * world_position.y)+floor(10 * world_position.z),2) == 0) {
		fragment_color = vec4(0.7,0.7,0.7,1);
	} else {
    fragment_color = vec4(0.1,0.1,0.1,1);
  }
}
