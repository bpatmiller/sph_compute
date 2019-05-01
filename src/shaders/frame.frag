#version 330 core
out vec4 fragment_color;

in vec2 uv;
uniform sampler2D frame_tex;

void main() { fragment_color = vec4(texture(frame_tex, uv).xyz, 1); }
