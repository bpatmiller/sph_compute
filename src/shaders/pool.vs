#version 430 core
in vec3 vertex_position;

uniform mat4 projection;
uniform mat4 view;

out vec3 world_position;

void main() { 
    world_position = vertex_position;
    gl_Position = projection * view * vec4(vertex_position, 1.0); }
