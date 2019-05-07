#version 330 core
out vec4 fragment_color;

in float scale;
in vec3 uv;

void main() {
  float eval_min = min(min(uv.x, uv.y), uv.z) * scale;
  if (eval_min < 0.005) {
    fragment_color = vec4(0.5, 0.5, 0.5, 1.0);
  } else {
    fragment_color = vec4(0);
  }
}
