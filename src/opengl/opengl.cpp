#include <iostream>
#include <fstream>
#include <sstream>
#include <numbers>
#include <utility>

#include <cstdlib>
#include <cmath>

#include <GL/glew.h>

#include <my-lib/math.h>

#include <my-game-lib/debug.h>
#include <my-game-lib/opengl/opengl.h>

// ---------------------------------------------------

#define DEBUG_SHOW_CENTER_LINE

// ---------------------------------------------------

namespace MyGlib
{
namespace Opengl
{

// ---------------------------------------------------

Shader::Shader (const GLenum shader_type_, const std::string_view fname_)
: shader_type(shader_type_),
  fname(fname_)
{
	this->shader_id = glCreateShader(this->shader_type);
}

void Shader::compile ()
{
	std::ifstream t(this->fname);
	std::stringstream str_stream;
	str_stream << t.rdbuf();
	std::string buffer = str_stream.str();

	dprintln("loaded shader (", this->fname, ")");
	//dprint( buffer )
	
	const char *c_str = buffer.c_str();
	glShaderSource(this->shader_id, 1, ( const GLchar ** )&c_str, nullptr);
	glCompileShader(this->shader_id);

	GLint status;
	glGetShaderiv(this->shader_id, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		GLint log_size = 0;
		glGetShaderiv(this->shader_id, GL_INFO_LOG_LENGTH, &log_size);

		std::vector<char> berror(log_size);
		glGetShaderInfoLog(this->shader_id, logSize, nullptr, berror.data());
		mylib_throw_exception_msg(this->fname, " shader compilation failed", '\n', berror.data());
	}
}

// ---------------------------------------------------

Program::Program ()
{
	this->vs = nullptr;
	this->fs = nullptr;
	this->program_id = glCreateProgram();
}

void Program::attach_shaders ()
{
	glAttachShader(this->program_id, this->vs->shader_id);
	glAttachShader(this->program_id, this->fs->shader_id);
}

void Program::link_program ()
{
	glLinkProgram(this->program_id);
}

void Program::use_program ()
{
	glUseProgram(this->program_id);
}

// ---------------------------------------------------

ProgramTriangle::ProgramTriangle ()
	: Program ()
{
	static_assert(sizeof(Vector) == sizeof(fp_t) * 3);
	static_assert(sizeof(Vector) == sizeof(Point));
	static_assert(sizeof(Color) == sizeof(float) * 4);
#ifndef OPENGL_SOFTWARE_CALCULATE_MATRIX
	static_assert(sizeof(Vertex) == (sizeof(Point) + sizeof(Vector) + sizeof(Color)));
#else
	static_assert(sizeof(Vertex) == (sizeof(Point4) + sizeof(Vector) + sizeof(Color)));
#endif

	this->vs = new Shader(GL_VERTEX_SHADER, "shaders/triangles.vert");
	this->vs->compile();

	this->fs = new Shader(GL_FRAGMENT_SHADER, "shaders/triangles.frag");
	this->fs->compile();

	this->attach_shaders();

	glBindAttribLocation(this->program_id, std::to_underlying(Attrib::Position), "i_position");
	glBindAttribLocation(this->program_id, std::to_underlying(Attrib::Offset), "i_offset");
	glBindAttribLocation(this->program_id, std::to_underlying(Attrib::Color), "i_color");

	this->link_program();

	glGenVertexArrays(1, &(this->vao));
	glGenBuffers(1, &(this->vbo));
}

void ProgramTriangle::bind_vertex_array ()
{
	glBindVertexArray(this->vao);
}

void ProgramTriangle::bind_vertex_buffer ()
{
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
}

void ProgramTriangle::setup_vertex_array ()
{
	uint32_t pos, length;

	glEnableVertexAttribArray( std::to_underlying(Attrib::Position) );
	glEnableVertexAttribArray( std::to_underlying(Attrib::Offset) );
	glEnableVertexAttribArray( std::to_underlying(Attrib::Color) );

	pos = 0;
#ifndef MYGLIB_OPENGL_SOFTWARE_CALCULATE_MATRIX
	length = 3;
#else
	length = 4;
#endif
	glVertexAttribPointer( std::to_underlying(Attrib::Position), length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
	
	pos += length;
	length = 3;
	glVertexAttribPointer( std::to_underlying(Attrib::Offset), length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
	
	pos += length;
	length = 4;
	glVertexAttribPointer( std::to_underlying(Attrib::Color), length, GL_FLOAT, GL_FALSE, sizeof(Vertex), ( void * )(pos * sizeof(float)) );
}

void ProgramTriangle::upload_vertex_buffer ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * n, this->triangle_buffer.get_vertex_buffer(), GL_DYNAMIC_DRAW);
}

void ProgramTriangle::upload_projection_matrix (const Matrix4& m)
{
	glUniformMatrix4fv( glGetUniformLocation(this->program_id, "u_projection_matrix"), 1, GL_TRUE, m.get_raw() );
	//dprintln( "projection matrix sent to GPU" )
}

void ProgramTriangle::draw ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();
	glDrawArrays(GL_TRIANGLES, 0, n);
}

void ProgramTriangle::debug ()
{
	const uint32_t n = this->triangle_buffer.get_vertex_buffer_used();

	for (uint32_t i=0; i<n; i++) {
		const Vertex& v = this->triangle_buffer.get_vertex(i);

		if ((i % 3) == 0)
			dprintln();

		dprintln("vertex[", i,
			"] x=", v.local_pos.x,
			" y=", v.local_pos.y,
			" z=", v.local_pos.z,
		#ifdef MYGLIB_OPENGL_SOFTWARE_CALCULATE_MATRIX
			" w=", v->local_pos.w,
		#endif
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

Renderer::Renderer (const InitParams& params)
	: GraphicsManager (params)
{
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	this->sdl_window = SDL_CreateWindow(
		params.window_name.data(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		this->window_width_px,
		this->window_height_px,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
		);

	this->sdl_gl_context = SDL_GL_CreateContext(this->sdl_window);

	GLenum err = glewInit();

	mylib_assert_exception_msg(err == GLEW_OK, "Error: ", glewGetErrorString(err))

	dprintln("Status: Using GLEW ", glewGetString(GLEW_VERSION));

	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);

	glClearColor(this->background_color.r, this->background_color.g, this->background_color.b, 1);
	glViewport(0, 0, this->window_width_px, this->window_height_px);

	this->load_opengl_programs();

	dprintln("loaded opengl stuff");

	this->wait_next_frame();
}

void Renderer::load_opengl_programs ()
{
	this->program_triangle = new ProgramTriangle;

	dprintln("loaded opengl triangle program");

	this->program_triangle->use_program();
	
	this->program_triangle->bind_vertex_array();
	this->program_triangle->bind_vertex_buffer();

	this->program_triangle->setup_vertex_array();

	dprintln("generated and binded opengl world vertex array/buffer");
}

Renderer::~Renderer ()
{
	delete this->program_triangle;

	SDL_GL_DeleteContext(this->sdl_gl_context);
	SDL_DestroyWindow(this->sdl_window);
}

void Renderer::wait_next_frame ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->program_triangle->clear();
}

void Renderer::draw_cube3D (const Cube3D& cube, const Vector& offset)
{
	const Vector local_pos = cube.get_value_delta();
	//const Vector world_pos = Vector(4.0f, 4.0f);

	using PositionIndex = Cube3d::PositionIndex;
	using enum PositionIndex;
	
#if 0
	dprint( "local_pos:" )
	Mylib::Math::println(world_pos);

	dprint( "clip_pos:" )
	Mylib::Math::println(clip_pos);
//exit(1);
#endif

	std::array<Point, 8> points;

	points[LeftTopFront] = Point(
		local_pos.x - cube.get_w()*fp(0.5),
		local_pos.y + cube.get_h()*fp(0.5),
		local_pos.z - cube.get_d()*fp(0.5)
		);
	
	points[LeftBottomFront] = Point(
		local_pos.x - cube.get_w()*fp(0.5),
		local_pos.y - cube.get_h()*fp(0.5),
		local_pos.z - cube.get_d()*fp(0.5)
		);
	
	points[RightTopFront] = Point(
		local_pos.x + cube.get_w()*fp(0.5),
		local_pos.y + cube.get_h()*fp(0.5),
		local_pos.z - cube.get_d()*fp(0.5)
		);
	
	points[RightBottomFront] = Point(
		local_pos.x + cube.get_w()*fp(0.5),
		local_pos.y - cube.get_h()*fp(0.5),
		local_pos.z - cube.get_d()*fp(0.5)
		);
	
	points[LeftTopBack] = Point(
		local_pos.x - cube.get_w()*fp(0.5),
		local_pos.y + cube.get_h()*fp(0.5),
		local_pos.z + cube.get_d()*fp(0.5)
		);
	
	points[LeftBottomBack] = Point(
		local_pos.x - cube.get_w()*fp(0.5),
		local_pos.y - cube.get_h()*fp(0.5),
		local_pos.z + cube.get_d()*fp(0.5)
		);
	
	points[RightTopBack] = Point(
		local_pos.x + cube.get_w()*fp(0.5),
		local_pos.y + cube.get_h()*fp(0.5),
		local_pos.z + cube.get_d()*fp(0.5)
		);
	
	points[RightBottomBack] = Point(
		local_pos.x + cube.get_w()*fp(0.5),
		local_pos.y - cube.get_h()*fp(0.5),
		local_pos.z + cube.get_d()*fp(0.5)
		);
	
	if (cube.get_rotation_angle() != fp(0)) {
		for (auto& p : points)
			p.rotate_around_axis(cube.get_ref_rotation_axis(), cube.get_rotation_angle());
	}

#ifdef MYGLIB_OPENGL_SOFTWARE_CALCULATE_MATRIX
	std::array<Point4, 8> points4;

	Matrix4 proj;
	proj.set_perspective(
		Mylib::Math::degrees_to_radians(fp(45)),
		static_cast<fp_t>(this->window_width_px),
		static_cast<fp_t>(this->window_height_px),
		fp(0.1),
		fp(100),
		fp(1)
	);
	dprintln("software projection matrix:", '\n', proj);
	for (int i = 0; auto& p : points) {
		Point translated = p + offset;
		Point4 p4 (translated.x, translated.y, translated.z, 1);
		//proj.set_identity();
		Point4 trans = proj * p4;
		//trans.w = 1;
		points4[i++] = trans;
		dprintln("trans ", trans);
	}
#endif
	
	constexpr uint32_t n_triangles = 12; // 6 faces * 2 triangles per face
	constexpr uint32_t n_vertices = n_triangles * 3;

	std::span<ProgramTriangle::Vertex> vertices = this->program_triangle->alloc_vertices(n_vertices);
	uint32_t i = 0;

#ifndef OPENGL_SOFTWARE_CALCULATE_MATRIX
	auto& points_ = points;
#else
	auto& points_ = points4;
#endif

	auto mount = [&i, vertices, &points_, &cube, &offset] (const PositionIndex p) -> void {
		vertices[i].local_pos = points_[p];
		vertices[i].offset = offset;
		vertices[i].color = cube.get_vertex_color(p);
		i++;
	};

	auto mount_triangle = [&mount] (const PositionIndex p1, const PositionIndex p2, const PositionIndex p3) -> void {
		mount(p1);
		mount(p2);
		mount(p3);
	};

	// bottom
	mount_triangle(LeftBottomFront, RightBottomFront, LeftBottomBack);
	mount_triangle(RightBottomBack, RightBottomFront, LeftBottomBack);

	// top
	mount_triangle(LeftTopFront, RightTopFront, LeftTopBack);
	mount_triangle(RightTopBack, RightTopFront, LeftTopBack);

	// front
	mount_triangle(LeftTopFront, LeftBottomFront, RightTopFront);
	mount_triangle(RightBottomFront, LeftBottomFront, RightTopFront);

	// back
	mount_triangle(LeftTopBack, LeftBottomBack, RightTopBack);
	mount_triangle(RightBottomBack, LeftBottomBack, RightTopBack);

	// left
	mount_triangle(LeftTopFront, LeftBottomFront, LeftTopBack);
	mount_triangle(LeftBottomBack, LeftBottomFront, LeftTopBack);

	// right
	mount_triangle(RightTopFront, RightBottomFront, RightTopBack);
	mount_triangle(RightBottomBack, RightBottomFront, RightTopBack);

	mylib_assert_exception(i == n_vertices)
}

void Renderer::setup_render_3D (const RenderArgs3D& args)
{
#ifndef MYGLIB_OPENGL_SOFTWARE_CALCULATE_MATRIX
	this->projection_matrix = Mylib::Math::gen_perspective_matrix<fp_t>(
			args.fov_y,
			static_cast<fp_t>(this->window_width_px),
			static_cast<fp_t>(this->window_height_px),
			args.z_near,
			args.z_far,
			fp(1)
		)
		* Mylib::Math::gen_look_at_matrix<fp_t>(
			args.world_camera_pos,
			args.world_camera_target,
			Vector(0, 1, 0));
#else
	this->projection_matrix = Mylib::Math::gen_identity_matrix<fp_t, 4>();
#endif

#if 1
	dprintln("projection matrix:");
	dprintln(this->projection_matrix);
	dprintln();
	dprintln("camera position: ", args.world_camera_pos);
	dprintln("camera target: ", args.world_camera_target);
	dprintln("camera vector: ", args.world_camera_target - args.world_camera_pos);
#endif
}

void Renderer::setup_render_2D (const RenderArgs2D& args)
{
#ifndef MYGLIB_OPENGL_SOFTWARE_CALCULATE_MATRIX
	const Vector2 normalized_clip_init = args.clip_init_norm;
	const Vector2 normalized_clip_end = args.clip_end_norm;

	const Vector2 normalized_clip_size = normalized_clip_end - normalized_clip_init;
	const fp_t normalized_clip_aspect_ratio = normalized_clip_size.x / normalized_clip_size.y;

	//const float max_norm_length = std::max(normalized_clip_size.x, normalized_clip_size.y);
	//const float max_opengl_length = max_norm_length * 2.0f;
	const fp_t opengl_length = 2;
	const fp_t opengl_window_aspect_ratio = this->window_aspect_ratio;

	/*
		1.0f (norm_length) -> 2.0f (opengl_length)
		norm_coord -> opengl_coord
	*/

	Vector2 opengl_clip_scale_mirror;

	if (normalized_clip_aspect_ratio >= fp(1))
		opengl_clip_scale_mirror = Vector2(opengl_length, opengl_length*opengl_window_aspect_ratio);
	else
		opengl_clip_scale_mirror = Vector2(opengl_length/opengl_window_aspect_ratio, opengl_length);
	
	// mirror y axis
	opengl_clip_scale_mirror.y = -opengl_clip_scale_mirror.y;

	const Vector2 world_size = args.world_end - args.world_init;
	
	const fp_t world_screen_width = std::min(args.world_screen_width, world_size.x);
	const fp_t world_screen_height = std::min(world_screen_width / normalized_clip_aspect_ratio, world_size.y);

	const Vector2 world_screen_size = Vector2(world_screen_width, world_screen_height);

	const fp_t normalized_scale_factor = normalized_clip_size.x / world_screen_size.x;
	//const float normalized_scale_factor = 1.0f / world_screen_size.x;

	Vector2 world_camera = args.world_camera_focus - Vector2(world_screen_size.x*fp(0.5), world_screen_size.y*fp(0.5));

	//dprintln( "------------------------------" )
	//dprint( "world_camera PRE: " ) Mylib::Math::println(world_camera);

	if (args.force_camera_inside_world) {
		if (world_camera.x < args.world_init.x)
			world_camera.x = args.world_init.x;
		else if ((world_camera.x + world_screen_size.x) > args.world_end.x)
			world_camera.x = args.world_end.x - world_screen_size.x;

		//dprint( "world_camera POS: " ) Mylib::Math::println(world_camera);

		if (world_camera.y < args.world_init.y)
			world_camera.y = args.world_init.y;
		else if ((world_camera.y + world_screen_size.y) > args.world_end.y)
			world_camera.y = args.world_end.y - world_screen_size.y;
	}

#if 0
	dprint( "normalized_clip_init: " ) Mylib::Math::println(normalized_clip_init);
	dprint( "normalized_clip_end: " ) Mylib::Math::println(normalized_clip_end);
	dprint( "normalized_clip_size: " ) Mylib::Math::println(normalized_clip_size);
	dprintln( "normalized_clip_aspect_ratio: " << normalized_clip_aspect_ratio )
	dprintln( "normalized_scale_factor: " << normalized_scale_factor )
	//dprintln( "max_norm_length: " << max_norm_length )
	//dprintln( "max_opengl_length: " << max_opengl_length )
	dprint( "opengl_clip_scale_mirror: " ) Mylib::Math::println(opengl_clip_scale_mirror);
	dprint( "world_size: " ) Mylib::Math::println(world_size);
	dprint( "world_screen_size: " ) Mylib::Math::println(world_screen_size);
	dprint( "args.world_camera_focus: " ) Mylib::Math::println(args.world_camera_focus);
	dprint( "world_camera: " ) Mylib::Math::println(world_camera);
//exit(1);
#endif

	// translate from (0, 2) to (-1, +1) opengl clip space
	Matrix4 translate_subtract_one;
	translate_subtract_one.set_translate( Vector(-1, +1) );
//	dprintln( "translation to clip init:" ) translate_to_clip_init.println();

	// mirror y axis
	// and also scale to (0, 2) coords
	Matrix4 opengl_scale_mirror;
	opengl_scale_mirror.set_scale(opengl_clip_scale_mirror);
//	dprintln( "scale matrix:" ) Mylib::Math::println(scale);
//exit(1);

	Matrix4 translate_to_normalized_clip_init;
	translate_to_normalized_clip_init.set_translate(normalized_clip_init);
//	dprintln( "translation to clip init:" ) translate_to_clip_init.println();

	Matrix4 scale_normalized;
	scale_normalized.set_scale(Vector(normalized_scale_factor, normalized_scale_factor));
//	dprintln( "scale matrix:" ) Mylib::Math::println(scale);
//exit(1);

	Matrix4 translate_camera;
	translate_camera.set_translate(-world_camera);
//	dprintln( "translation matrix:" ) translate_camera.println();

	this->projection_matrix = 
		(((translate_subtract_one
		* opengl_scale_mirror)
		* translate_to_normalized_clip_init)
		* scale_normalized)
		* translate_camera;
	//this->projection_matrix = scale * translate_camera;
	//dprintln( "final matrix:" ) this->projection_matrix.println();
#else
	this->projection_matrix = Mylib::Math::gen_identity_matrix<fp_t, 4>();
#endif

#if 1
	dprintln("projection matrix:");
	dprintln(this->projection_matrix);
	dprintln();
	dprintln("camera position: ", args.world_camera_pos);
	dprintln("camera target: ", args.world_camera_target);
	dprintln("camera vector: ", args.world_camera_target - args.world_camera_pos);
#endif
}

void Renderer::render ()
{
	//this->program_triangle->debug();
	this->program_triangle->upload_projection_matrix(this->projection_matrix);
	this->program_triangle->upload_vertex_buffer();
	this->program_triangle->draw();
	SDL_GL_SwapWindow(this->sdl_window);
}

// ---------------------------------------------------

} // namespace Opengl
} // namespace MyGlib