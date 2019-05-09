#version 430 core
out vec4 fragment_color;

in float dens;
in float pres;

uniform int color_mode;

void main() {
  // 1 - density
  if (color_mode == 1) {
    fragment_color =
        mix(vec4(0.1, 0.7, 0.3, 1), vec4(0.2, 0.4, 0.65, 1), dens / 200000.0);
    // 2 - pressure
  } else if (color_mode == 2) {
    fragment_color =
        mix(vec4(0.6, 0.6, 0.5, 1), vec4(0.8, 0.2, 0.65, 1), 20 * pres);
    // 3 - other??
  } else {
    fragment_color =
        mix(vec4(0.1, 0.7, 0.3, 1), vec4(0.2, 0.4, 0.65, 1), dens / 200000.0);
  }
}
