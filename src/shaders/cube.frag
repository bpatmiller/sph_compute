#version 430 core
out vec4 fragment_color;

in float dens;
in float pres;
in float has;
in float _p0;
in float col;
in vec3 f;

uniform int num_cells;
uniform int color_mode;

void main() {
  // 1 - density
  if (color_mode == 1) {
    fragment_color =
        mix(vec4(0.1, 0.7, 0.3, 1), vec4(0.2, 0.4, 0.65, 1), dens / 100000.0);
    // 2 - pressure
  } else if (color_mode == 2) {
    fragment_color =
        mix(vec4(0.6, 0.6, 0.5, 1), vec4(0.8, 0.2, 0.65, 1), 40 * pres);
    // 3 - hash
  } else if (color_mode == 3) {
    fragment_color = mix(vec4(0.7, 0.7, 0.2, 1), vec4(0.2, 0.2, 0.8, 1),
                         float(has) / float(num_cells));
    // 4 - hash - to - index
  } else if (color_mode == 4) {
    fragment_color = mix(vec4(0.1, 0.0, 0.3, 1), vec4(0.2, 0.4, 0.65, 1),
                         float(_p0) / float(num_cells));
  }
  // forces
  else if (color_mode == 5) {
    fragment_color = vec4(f * 0.0001, 1.0);
  } else {
    fragment_color = vec4(mix(vec3(0), vec3(1), col), 1.0);
  }
}