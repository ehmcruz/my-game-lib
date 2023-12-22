#include <iostream>
#include <fstream>
#include <sstream>
#include <numbers>
#include <utility>

#include <cstdlib>
#include <cmath>

#include <my-lib/math.h>

#include <my-game-lib/debug.h>
#include <my-game-lib/opengl/opengl.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{
namespace Opengl
{

// ---------------------------------------------------

static void ensure_no_error ()
{
	const GLenum error = glGetError();
	mylib_assert_exception_msg(error == GL_NO_ERROR, "glGetError() returned ", error);
}

// ---------------------------------------------------

Shader::Shader (const GLenum shader_type_, const std::string_view fname_)
: shader_type(shader_type_),
  fname(fname_)
{
	this->shader_id = glCreateShader(this->shader_type);
	mylib_assert_exception_msg(this->shader_id != 0, "glCreateShader failed");
}

Shader::~Shader ()
{

}

void Shader::compile ()
{
	// Using SDL to load the file because it automatically
	// handles platform-specific file paths, specially on Android.

	SDL_RWops *fp = SDL_RWFromFile(this->fname.data(), "rb");
	mylib_assert_exception_msg(fp != nullptr, "SDL_RWFromFile failed");

	const Sint64 fsize = SDL_RWseek(fp, 0, RW_SEEK_END);
	mylib_assert_exception_msg(fsize != -1, "SDL_RWseek failed");

	const Sint64 fseekerror = SDL_RWseek(fp, 0, RW_SEEK_SET);
	mylib_assert_exception_msg(fseekerror != -1, "SDL_RWseek failed on returnign to start of file");

	std::vector<char> buffer(fsize + 1);

	const size_t nread = SDL_RWread(fp, buffer.data(), sizeof(char), fsize);
	mylib_assert_exception_msg(static_cast<Sint64>(nread) == fsize, "SDL_RWread failed nread=", nread, " fsize=", fsize);

	buffer[fsize] = 0;

	SDL_RWclose(fp);

#if 0
	// Old code. Deprecated because it doesn't work on Android.

	std::ifstream t(this->fname);
	std::stringstream str_stream;
	str_stream << t.rdbuf();
	std::string buffer = str_stream.str();
#endif

	dprintln("loaded shader (", this->fname, ")");
	//dprint( buffer )
	
	const char *c_str = buffer.data();
	glShaderSource(this->shader_id, 1, ( const GLchar ** )&c_str, nullptr);
	ensure_no_error();
	glCompileShader(this->shader_id);

	GLint status;
	glGetShaderiv(this->shader_id, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		GLint log_size = 0;
		glGetShaderiv(this->shader_id, GL_INFO_LOG_LENGTH, &log_size);

		std::vector<char> berror(log_size);
		glGetShaderInfoLog(this->shader_id, log_size, nullptr, berror.data());
		mylib_throw_exception_msg(this->fname, " shader compilation failed", '\n', berror.data());
	}
}

// ---------------------------------------------------

Program::Program ()
{
	this->vs = nullptr;
	this->fs = nullptr;
	this->program_id = glCreateProgram();
	mylib_assert_exception_msg(this->program_id != 0, "glCreateProgram failed");
}

Program::~Program ()
{
	if (this->vs != nullptr)
		delete this->vs;
	if (this->fs != nullptr)
		delete this->fs;
	this->vs = nullptr;
	this->fs = nullptr;
}

void Program::attach_shaders ()
{
	glAttachShader(this->program_id, this->vs->get_shader_id());
	ensure_no_error();
	glAttachShader(this->program_id, this->fs->get_shader_id());
	ensure_no_error();
}

void Program::link_program ()
{
	glLinkProgram(this->program_id);
	ensure_no_error();
}

void Program::use_program ()
{
	glUseProgram(this->program_id);
	ensure_no_error();
}

GLint Program::get_uniform_location (const std::string_view name) const
{
	const GLint location = glGetUniformLocation(this->program_id, name.data());
	mylib_assert_exception_msg(location != -1, "uniform ", name, " not found");
	return location;
}

void Program::bind_attrib_location (const GLuint index, const std::string_view name)
{
	glBindAttribLocation(this->program_id, index, name.data());
	ensure_no_error();
}

void Program::gen_vertex_arrays (const GLsizei n, GLuint *arrays)
{
	glGenVertexArrays(n, arrays);
	ensure_no_error();
}

void Program::gen_buffers (const GLsizei n, GLuint *buffers)
{
	glGenBuffers(n, buffers);
	ensure_no_error();
}

void Program::bind_vertex_array (const GLuint array)
{
	glBindVertexArray(array);
	ensure_no_error();
}

void Program::bind_buffer (const GLenum target, const GLuint buffer)
{
	glBindBuffer(target, buffer);
	ensure_no_error();
}

void Program::enable_vertex_attrib_array (const GLuint index)
{
	glEnableVertexAttribArray(index);
	ensure_no_error();
}

// ---------------------------------------------------

ProgramTriangleColor::ProgramTriangleColor ()
	: Program ()
{
	static_assert(sizeof(Graphics::Vertex) == sizeof(Point) + sizeof(Vector));
	static_assert(sizeof(Vector) == sizeof(fp_t) * 3);
	static_assert(sizeof(Vector) == sizeof(Point));
	static_assert(sizeof(Color) == sizeof(float) * 4);
	static_assert(sizeof(Vertex) == (sizeof(Graphics::Vertex) + sizeof(Vector) + sizeof(Color)));

	this->vs = new Shader(GL_VERTEX_SHADER, "shaders/triangles-color.vert");
	this->vs->compile();

	this->fs = new Shader(GL_FRAGMENT_SHADER, "shaders/triangles-color.frag");
	this->fs->compile();

	this->attach_shaders();

	this->bind_attrib_location(iPosition, "i_position");
	this->bind_attrib_location(iNormal, "i_normal");
	this->bind_attrib_location(iOffset, "i_offset");
	this->bind_attrib_location(iColor, "i_color");

	this->link_program();

	this->gen_vertex_arrays(1, &(this->vao));
	this->gen_buffers(1, &(this->vbo));

	this->use_program();
	this->bind_vertex_arrays();
	this->bind_vertex_buffers();
	this->setup_vertex_arrays();
	this->setup_uniforms();

	dprintln("loaded opengl triangle color program");
}

ProgramTriangleColor::~ProgramTriangleColor ()
{

}

void ProgramTriangleColor::bind_vertex_arrays ()
{
	this->bind_vertex_array(this->vao);
}

void ProgramTriangleColor::bind_vertex_buffers ()
{
	this->bind_buffer(GL_ARRAY_BUFFER, this->vbo);
}

void ProgramTriangleColor::setup_vertex_arrays ()
{
	uint32_t pos, length;

	this->enable_vertex_attrib_array(iPosition);
	this->enable_vertex_attrib_array(iNormal);
	this->enable_vertex_attrib_array(iOffset);
	this->enable_vertex_attrib_array(iColor);

	pos = 0;
	length = 3;
	glVertexAttribPointer(iPosition, length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );

	pos += length;
	length = 3;
	glVertexAttribPointer(iNormal, length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );

	pos += length;
	length = 3;
	glVertexAttribPointer(iOffset, length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
	
	pos += length;
	length = 4;
	glVertexAttribPointer(iColor, length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );

	ensure_no_error();
}

void ProgramTriangleColor::setup_uniforms ()
{
	this->u_projection_matrix = this->get_uniform_location("u_projection_matrix");
	this->u_ambient_light_color = this->get_uniform_location("u_ambient_light_color");
	this->u_point_light_pos = this->get_uniform_location("u_point_light_pos");
	this->u_point_light_color = this->get_uniform_location("u_point_light_color");
}

void ProgramTriangleColor::upload_vertex_buffers ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * n, this->triangle_buffer.get_vertex_buffer(), GL_DYNAMIC_DRAW);

	ensure_no_error();
}

void ProgramTriangleColor::upload_uniforms (const Uniforms& uniforms)
{
	glUniformMatrix4fv(this->u_projection_matrix, 1, GL_TRUE, uniforms.projection_matrix.get_raw());
	glUniform4fv(this->u_ambient_light_color, 1, uniforms.ambient_light_color.get_raw());
	glUniform3fv(this->u_point_light_pos, 1, uniforms.point_light_pos.get_raw());
	glUniform4fv(this->u_point_light_color, 1, uniforms.point_light_color.get_raw());

	ensure_no_error();
}

void ProgramTriangleColor::draw ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();
	glDrawArrays(GL_TRIANGLES, 0, n);

	ensure_no_error();
}

void ProgramTriangleColor::load ()
{
	this->use_program();
	this->bind_vertex_arrays();
	this->bind_vertex_buffers();
}

void ProgramTriangleColor::debug ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();

	for (uint32_t i=0; i<n; i++) {
		const Vertex& v = this->triangle_buffer.get_vertex(i);

		if ((i % 3) == 0)
			dprintln();

		dprintln("vertex[", i,
			"] x=", v.gvertex.pos.x,
			" y=", v.gvertex.pos.y,
			" z=", v.gvertex.pos.z,
			" offset_x=", v.offset.x,
			" offset_y=", v.offset.y,
			" offset_z=", v.offset.z,
			" r=", v.color.r,
			" g=", v.color.g,
			" b=", v.color.b,
			" a=", v.color.a
		);
	}
}

// ---------------------------------------------------

ProgramTriangleTexture::ProgramTriangleTexture ()
	: Program ()
{
	static_assert(sizeof(Graphics::Vertex) == sizeof(Point) + sizeof(Vector));
	static_assert(sizeof(Vector) == sizeof(fp_t) * 3);
	static_assert(sizeof(Vector) == sizeof(Point));
	static_assert(sizeof(Color) == sizeof(float) * 4);
	static_assert(sizeof(Vertex) == (sizeof(Graphics::Vertex) + sizeof(Vector) + sizeof(Vector2f)));

	this->vs = new Shader(GL_VERTEX_SHADER, "shaders/triangles-texture.vert");
	this->vs->compile();

	this->fs = new Shader(GL_FRAGMENT_SHADER, "shaders/triangles-texture.frag");
	this->fs->compile();

	this->attach_shaders();

	this->bind_attrib_location(iPosition, "i_position");
	this->bind_attrib_location(iNormal, "i_normal");
	this->bind_attrib_location(iOffset, "i_offset");
	this->bind_attrib_location(iTexCoords, "i_tex_coord");

	this->link_program();

	this->gen_vertex_arrays(1, &(this->vao));
	this->gen_buffers(1, &(this->vbo));

	this->use_program();
	this->bind_vertex_arrays();
	this->bind_vertex_buffers();
	this->setup_vertex_arrays();
	this->setup_uniforms();

	dprintln("loaded opengl triangle texture program");
}

ProgramTriangleTexture::~ProgramTriangleTexture ()
{

}

void ProgramTriangleTexture::bind_vertex_arrays ()
{
	this->bind_vertex_array(this->vao);
}

void ProgramTriangleTexture::bind_vertex_buffers ()
{
	this->bind_buffer(GL_ARRAY_BUFFER, this->vbo);
}

void ProgramTriangleTexture::setup_vertex_arrays ()
{
	uint32_t pos, length;

	this->enable_vertex_attrib_array(iPosition);
	this->enable_vertex_attrib_array(iNormal);
	this->enable_vertex_attrib_array(iOffset);
	this->enable_vertex_attrib_array(iTexCoords);

	pos = 0;
	length = 3;
	glVertexAttribPointer(iPosition, length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );

	pos += length;
	length = 3;
	glVertexAttribPointer(iNormal, length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );

	pos += length;
	length = 3;
	glVertexAttribPointer(iOffset, length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
	
	pos += length;
	length = 2;
	glVertexAttribPointer(iTexCoords, length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );

	ensure_no_error();
}

void ProgramTriangleTexture::setup_uniforms ()
{
	this->u_projection_matrix = this->get_uniform_location("u_projection_matrix");
	this->u_ambient_light_color = this->get_uniform_location("u_ambient_light_color");
	this->u_point_light_pos = this->get_uniform_location("u_point_light_pos");
	this->u_point_light_color = this->get_uniform_location("u_point_light_color");
	this->u_tx_unit = this->get_uniform_location("u_tx_unit");
}

void ProgramTriangleTexture::upload_vertex_buffers ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * n, this->triangle_buffer.get_vertex_buffer(), GL_DYNAMIC_DRAW);

	ensure_no_error();
}

void ProgramTriangleTexture::upload_uniforms (const Uniforms& uniforms)
{
	glUniformMatrix4fv(this->u_projection_matrix, 1, GL_TRUE, uniforms.projection_matrix.get_raw());
	glUniform4fv(this->u_ambient_light_color, 1, uniforms.ambient_light_color.get_raw());
	glUniform3fv(this->u_point_light_pos, 1, uniforms.point_light_pos.get_raw());
	glUniform4fv(this->u_point_light_color, 1, uniforms.point_light_color.get_raw());
	glUniform1i(this->u_tx_unit, 0); // set shader to use texture unit 0

	ensure_no_error();
}

void ProgramTriangleTexture::draw ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();
	glDrawArrays(GL_TRIANGLES, 0, n);

	ensure_no_error();
}

void ProgramTriangleTexture::load ()
{
	this->use_program();
	this->bind_vertex_arrays();
	this->bind_vertex_buffers();
}

void ProgramTriangleTexture::debug ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();

	for (uint32_t i=0; i<n; i++) {
		const Vertex& v = this->triangle_buffer.get_vertex(i);

		if ((i % 3) == 0)
			dprintln();

		dprintln("vertex[", i,
			"] x=", v.gvertex.pos.x,
			" y=", v.gvertex.pos.y,
			" z=", v.gvertex.pos.z,
			" offset_x=", v.offset.x,
			" offset_y=", v.offset.y,
			" offset_z=", v.offset.z,
			" tex_x=", v.tex_coords.x,
			" tex_y=", v.tex_coords.y
		);
	}
}

// ---------------------------------------------------

} // namespace Graphics
} // namespace Opengl
} // namespace MyGlib