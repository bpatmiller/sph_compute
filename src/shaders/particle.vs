#version 430 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 position;
layout(location = 2) in float density;
layout(location = 3) in vec3 velocity;
layout(location = 4) in float pressure;
layout(location = 5) in vec3 acceleration;
layout(location = 6) in float hash;
layout(location = 7) in vec3 force;
layout(location = 8) in float pad0;
layout(location = 9) in vec3 normal;
layout(location = 10) in float pad1;

uniform mat4 projection;
uniform mat4 view;

out vec3 v_color;
uniform int num_cells;
uniform int color_mode;

void main() {

  gl_Position = projection * view * vec4(position + vertex_position, 1.0);

  // 1 - density
  if (color_mode == 1) {
    v_color = mix(vec3(0.1, 0.7, 0.3), vec3(0.2, 0.4, 0.65), density / 50000.0);
    // 2 - pressure
  } else if (color_mode == 2) {
    v_color = mix(vec3(0.6, 0.6, 0.5), vec3(0.8, 0.2, 0.65), 200 * pressure);
    // 3 - hash
  } else if (color_mode == 3) {
    v_color = mix(vec3(0.7, 0.7, 0.2), vec3(0.2, 0.2, 0.8),
                  float(hash) / float(num_cells));
    // 4 - acceleration
  } else if (color_mode == 4) {
    v_color = 0.05 * abs(acceleration);
  }
  // forces
  else if (color_mode == 5) {
    v_color = normalize(abs(force)); // clamp(abs(force) * 0.00001, 0, 1);
  }
  // starting x position
  else if (color_mode == 6) {
    // v_color = mix(vec3(0.1, 0.7, 0.3), vec3(0.2, 0.4, 0.65), pad1);
    v_color = mix(vec3(0), vec3(1), pad1);
  }
  // normals
  else if (color_mode == 7) {
    // v_color = normalize(abs(normal));
    float n = normalize(normal).y;
    v_color = mix(vec3(0.6, 0.6, 0.6), vec3(0.8, 0.1, 0.1), n);
  }
  // velocity
  else if (color_mode == 8) {
    v_color = vec3(0.25) + 0.7 * abs(velocity);
  }
  // SPEED
  else {
    // v_color =
    //     mix(vec3(0.6, 0.6, 0.6), vec3(0.8, 0.1, 0.1), length(velocity) *
    //     0.25);
    v_color =
        mix(vec3(0.2, 0.4, 0.65), vec3(0.1, 0.7, 0.3), length(velocity) * 0.5);
  }
}
