#version 300 es

in vec3 i_position;
in vec3 i_normal;
in vec3 i_offset;
in vec3 i_tex_coord;
in vec4 i_rot_quat;

out vec3 world_position;
out vec3 normal;
out vec3 tex_coord;

uniform mat4 u_projection_matrix;

vec4 quaternion_mul (const vec4 q1, const vec4 q2)
{
	vec4 r;

	r.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
	r.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
	r.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
	r.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);

	return r;
}

vec4 quaternion_conjugate (const vec4 q)
{
	return vec4(-q.x, -q.y, -q.z, q.w);
}

vec3 rotate (const vec4 q, const vec3 v)
{
	vec4 tmp = quaternion_mul(q, vec4(v, 0));
	vec4 r = quaternion_mul(tmp, quaternion_conjugate(q));

	return r.xyz;

	//return v + 2.0 * cross(cross(v, q.xyz ) + q.w * v, q.xyz);
}

void main ()
{
	tex_coord = i_tex_coord;
	world_position = rotate(i_rot_quat, i_position) + i_offset;
	normal = normalize(rotate(i_rot_quat, i_normal));
	gl_Position = u_projection_matrix * vec4(world_position, 1.0 );
}