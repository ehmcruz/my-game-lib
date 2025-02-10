#version 300 es

in vec3 i_position;
in vec3 i_direction; // This is the direction of the line
in vec3 i_offset;
in vec4 i_color;

out vec3 world_position;
out vec3 direction;
out vec4 color;

uniform mat4 u_projection_matrix;

void main ()
{
	color = i_color;
	world_position = i_position + i_offset;
	direction = normalize(i_direction);
	gl_Position = u_projection_matrix * vec4(world_position, 1.0 );
}