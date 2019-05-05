#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 light_position;
uniform vec3 camera_position;

in float[] d;

out vec3 normal;
out vec3 light_direction;
out vec3 view_direction;
out vec3 triangle_coords;
out float scale;
out float density;

void main()
{
    vec3 A = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 B = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	scale = length(B);
	normal = normalize(cross(B,A));

    mat4 mvp = projection * view * model;

	for (int n = 0; n < gl_in.length(); n++) {
		view_direction = camera_position - gl_in[n].gl_Position.xyz;
        light_direction = light_position - gl_in[n].gl_Position.xyz;
		gl_Position = mvp * gl_in[n].gl_Position;
		triangle_coords = vec3( 0.0, 0.0, 0.0 );
		triangle_coords[n] = 1.0;
		density = d[n];
		EmitVertex();
	}
	EndPrimitive();
}