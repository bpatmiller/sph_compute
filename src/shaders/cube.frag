#version 330 core
out vec4 fragment_color;

// in float d;

void main() {
  fragment_color =
      mix(vec4(0.1, 0.7, 0.3, 1), vec4(0.2, 0.4, 0.65, 1), 0.5); // d + 0.1);
}
