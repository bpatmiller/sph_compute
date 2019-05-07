#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection;
uniform mat4 view;

out float scale;
out vec3 uv;

void main()
{
    vec3 A = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	scale = length(A);

    mat4 mvp = projection * view;

	for (int n = 0; n < gl_in.length(); n++) {
		gl_Position = mvp * gl_in[n].gl_Position;
		uv = vec3( 0.0, 0.0, 0.0 );
		uv[n] = 1.0;
		EmitVertex();
	}
	EndPrimitive();
}