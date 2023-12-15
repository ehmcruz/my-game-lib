#version 330

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