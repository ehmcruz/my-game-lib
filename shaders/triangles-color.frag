#version 300 es

/*
	"precision" is required by OpenGL ES 3.0.

	https://stackoverflow.com/questions/13780609/what-does-precision-mediump-float-mean

	In this stackoverflow answer, it says that:
	- highp for vertex positions;
	- mediump for texture coordinates;
	- lowp for colors.

	It also says that highp is not always available, so let's use mediump.
*/
precision mediump float;

in vec3 world_position;
in vec3 normal;
in vec4 color;

out vec4 o_color;

uniform vec4 u_ambient_light_color;

uniform vec3 u_point_light_pos;
uniform vec4 u_point_light_color;

void main ()
{
	vec3 light_dir = normalize(u_point_light_pos - world_position);
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse_light = u_point_light_color.rgb * diff * u_point_light_color.a;

	vec3 ambient_light = u_ambient_light_color.rgb * u_ambient_light_color.a;

	vec3 result = (ambient_light + diffuse_light) * color.rgb;
	o_color = vec4(result, color.a);
}