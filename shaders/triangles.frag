#version 330

in vec4 v_color;

out vec4 o_color;

uniform vec4 u_ambient_light;

void main ()
{
	o_color = v_color * u_ambient_light;
}