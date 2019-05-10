#version 330 core
out vec4 fragment_color;

in vec2 uv;
uniform sampler2D screenTex;
uniform sampler2D depTex;
uniform vec2 frameBufSize;
uniform int renderMode;

void main() {
  if (renderMode == 1) {
    fragment_color = texture(screenTex, uv);
  } else {
    float dep = texture(depTex, uv).x;
    fragment_color = vec4(100.0 * (1.0 - dep), 0, 0, 1);
  }
}
