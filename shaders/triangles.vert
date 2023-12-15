#version 330

in vec3 i_position;
in vec3 i_normal;
in vec3 i_offset;
in vec4 i_color;

out vec3 world_position;
out vec3 normal;
out vec4 color;

uniform mat4 u_projection_matrix;

void main ()
{
	color = i_color;
	world_position = i_position + i_offset;
	//normal = normalize(i_normal);
	normal = i_normal;
	gl_Position = u_projection_matrix * vec4(world_position, 1.0 );

	//gl_Position = i_position;
}