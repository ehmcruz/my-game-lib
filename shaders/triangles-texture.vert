#version 300 es

in vec3 i_position;
in vec3 i_normal;
in vec3 i_offset;
in vec3 i_tex_coord;

out vec3 world_position;
out vec3 normal;
out vec3 tex_coord;

uniform mat4 u_projection_matrix;

void main ()
{
	tex_coord = i_tex_coord;
	world_position = i_position + i_offset;
	normal = normalize(i_normal);
	gl_Position = u_projection_matrix * vec4(world_position, 1.0 );
}