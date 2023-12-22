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

#define DEBUG_SHOW_CENTER_LINE

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{
namespace Opengl
{

// ---------------------------------------------------

Renderer::Renderer (const InitParams& params)
	: Manager (params)
{
//	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // already set in sdl.cpp
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );


	// Check sdl.cpp
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	if (this->fullscreen) {
		SDL_DisplayMode display_mode;

		const auto error = SDL_GetCurrentDisplayMode(0, &display_mode);
		
		mylib_assert_exception_msg(error == 0, "error getting display mode\n", SDL_GetError())

		this->window_width_px = display_mode.w;
		this->window_height_px = display_mode.h;

		this->sdl_window = SDL_CreateWindow(
			params.window_name.data(),
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			this->window_width_px, this->window_height_px,
			SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
		);

		dprintln("fullscrren window created with width=", this->window_width_px, " height=", this->window_height_px);
	}
	else
		this->sdl_window = SDL_CreateWindow(
			params.window_name.data(),
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			this->window_width_px, this->window_height_px,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	this->sdl_gl_context = SDL_GL_CreateContext(this->sdl_window);

#ifndef __ANDROID__
	GLenum err = glewInit();

	mylib_assert_exception_msg(err == GLEW_OK, "Error: ", glewGetErrorString(err))

	dprintln("Status: Using GLEW ", glewGetString(GLEW_VERSION));
#endif

	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);

	glClearColor(this->background_color.r, this->background_color.g, this->background_color.b, 1);
	glViewport(0, 0, this->window_width_px, this->window_height_px);

	this->load_opengl_programs();

	dprintln("Opengl renderer created");

	this->wait_next_frame();
}

// ---------------------------------------------------

void Renderer::load_opengl_programs ()
{
	dprintln("loading opengl programs...");

	this->program_triangle_color = new ProgramTriangleColor;
	this->program_triangle_texture = new ProgramTriangleTexture;

	dprintln("all opengl programs loaded");
}

// ---------------------------------------------------

Renderer::~Renderer ()
{
	delete this->program_triangle_color;
	delete this->program_triangle_texture;

	SDL_GL_DeleteContext(this->sdl_gl_context);
	SDL_DestroyWindow(this->sdl_window);
}

// ---------------------------------------------------

void Renderer::wait_next_frame ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	this->clear_vertex_buffers();
}

// ---------------------------------------------------

void Renderer::draw_cube3D (Cube3D& cube, const Vector& offset, const Color& color)
{
	constexpr uint32_t n_vertices = Cube3D::get_n_vertices();
	//const Vector world_pos = Vector(4.0f, 4.0f);

#if 0
	dprint( "local_pos:" )
	Mylib::Math::println(world_pos);

	//dprint( "clip_pos:" )
	//Mylib::Math::println(clip_pos);
//exit(1);
#endif

	std::span<ProgramTriangleColor::Vertex> vertices = this->program_triangle_color->alloc_vertices(n_vertices);
	std::span<Vertex> shape_vertices = cube.get_local_rotated_vertices();

/*	dprintln("rendering cube with offset=", offset, " color=", color, " w=", cube.get_w(), " h=", cube.get_h(), " d=", cube.get_d());
	for (const auto& v : shape_vertices) { Vector4 trans = this->uniforms.projection_matrix * Vector4(v.pos.x+offset.x, v.pos.y+offset.y, v.pos.z+offset.z, 1); trans /= trans.w;
		dprintln("\tvertex.pos: ", v.pos, " transformed.pos: ", trans);
		//dprintln("\tvertex.normal: ", v.normal);
		}*/

	mylib_assert_exception(shape_vertices.size() == n_vertices)

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex = shape_vertices[i];
		vertices[i].offset = offset;
		vertices[i].color = color;
	}
}

// ---------------------------------------------------

void Renderer::draw_sphere3D (Sphere3D& sphere, const Vector& offset, const Color& color)
{
	const uint32_t n_vertices = sphere.get_n_vertices();
	std::span<Vertex> shape_vertices = sphere.get_local_rotated_vertices();

	mylib_assert_exception(shape_vertices.size() == n_vertices)

	//dprintln("circle_size_per_cent_of_screen: ", circle_size_per_cent_of_screen, " n_triangles: ", n_vertices / 3);

	std::span<ProgramTriangleColor::Vertex> vertices = this->program_triangle_color->alloc_vertices(n_vertices);

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex = shape_vertices[i];
		vertices[i].offset = offset;
		vertices[i].color = color;
	}
}

// ---------------------------------------------------

void Renderer::draw_circle2D (Circle2D& circle, const Vector& offset, const Color& color)
{
	/*Graphics::ShapeRect rect(circle.get_radius()*2.0f, circle.get_radius()*2.0f);
	rect.set_delta(circle.get_delta());
	this->draw_rect(rect, offset, color);*/

	// let's first estimate the size of the circle on the screen
	// and then we can calculate the number of vertices needed to draw it

	//circle.calculate_vertices(this->projection_matrix);
	
	const uint32_t n_vertices = circle.get_n_vertices();
	std::span<Vertex> shape_vertices = circle.get_local_rotated_vertices();

	mylib_assert_exception(shape_vertices.size() == n_vertices)

	//dprintln("circle_size_per_cent_of_screen: ", circle_size_per_cent_of_screen, " n_triangles: ", n_vertices / 3);

	std::span<ProgramTriangleColor::Vertex> vertices = this->program_triangle_color->alloc_vertices(n_vertices);

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex = shape_vertices[i];
		vertices[i].offset = offset;
		vertices[i].color = color;
	}
}

// ---------------------------------------------------

void Renderer::draw_rect2D (Rect2D& rect, const Vector& offset, const Color& color)
{
	constexpr uint32_t n_vertices = Rect2D::get_n_vertices();
	//const Vector world_pos = Vector(4.0f, 4.0f);
	
#if 0
	dprint( "local_pos:" )
	Mylib::Math::println(world_pos);

	dprint( "clip_pos:" )
	Mylib::Math::println(clip_pos);
//exit(1);
#endif

	std::span<ProgramTriangleColor::Vertex> vertices = this->program_triangle_color->alloc_vertices(n_vertices);
	std::span<Vertex> shape_vertices = rect.get_local_rotated_vertices();

	mylib_assert_exception(shape_vertices.size() == n_vertices)

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex = shape_vertices[i];
		vertices[i].offset = offset;
		vertices[i].color = color;
	}
}

void Renderer::draw_rect2D (Rect2D& rect, const Vector& offset, const TextureDescriptor& texture_desc)
{
	//mylib_throw_exception_msg("not implemented");
}

// ---------------------------------------------------

void Renderer::setup_render_3D (const RenderArgs3D& args)
{
	this->program_triangle_color_uniforms.projection_matrix = Mylib::Math::gen_perspective_matrix<fp_t>(
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

	this->program_triangle_color_uniforms.ambient_light_color = args.ambient_light_color;

	this->program_triangle_texture_uniforms.projection_matrix = this->program_triangle_color_uniforms.projection_matrix;
	this->program_triangle_texture_uniforms.ambient_light_color = this->program_triangle_color_uniforms.ambient_light_color;

#if 0
	dprintln("projection matrix:");
	dprintln(this->uniforms.projection_matrix);
	dprintln();
	dprintln("camera position: ", args.world_camera_pos);
	dprintln("camera target: ", args.world_camera_target);
	dprintln("camera vector: ", args.world_camera_target - args.world_camera_pos);
	dprintln("znear: ", args.z_near);
	dprintln("zfar: ", args.z_far);
	dprintln("ambient light color: ", this->uniforms.ambient_light_color);
	dprintln("point light pos: ", this->uniforms.point_light_pos);
	dprintln("point light color: ", this->uniforms.point_light_color);
#endif
}

// ---------------------------------------------------

void Renderer::setup_render_2D (const RenderArgs2D& args)
{
#ifndef MYGLIB_OPENGL_SOFTWARE_CALCULATE_MATRIX
	const Vector2 normalized_clip_init = args.clip_init_norm;
	const Vector2 normalized_clip_end = args.clip_end_norm;

	const Vector2 normalized_clip_size = normalized_clip_end - normalized_clip_init;
	const fp_t normalized_clip_aspect_ratio = normalized_clip_size.x / normalized_clip_size.y;

	//const float max_norm_length = std::max(normalized_clip_size.x, normalized_clip_size.y);
	//const float max_opengl_length = max_norm_length * 2.0f;
	constexpr fp_t opengl_length = 2;
	const fp_t opengl_window_aspect_ratio = this->get_window_aspect_ratio();

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
	translate_subtract_one.set_translate( Vector2(-1, +1) );
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
	scale_normalized.set_scale(Vector2(normalized_scale_factor, normalized_scale_factor));
//	dprintln( "scale matrix:" ) Mylib::Math::println(scale);
//exit(1);

	Matrix4 translate_camera;
	translate_camera.set_translate(-world_camera);
//	dprintln( "translation matrix:" ) translate_camera.println();

	this->program_triangle_color_uniforms.projection_matrix = 
		(((translate_subtract_one
		* opengl_scale_mirror)
		* translate_to_normalized_clip_init)
		* scale_normalized)
		* translate_camera;
	//this->projection_matrix = scale * translate_camera;
	//dprintln( "final matrix:" ) this->projection_matrix.println();

	this->program_triangle_color_uniforms.ambient_light_color = {1, 1, 1, 1};

	this->light_point_sources[0].pos = Vector3(0, 0, -1);
	this->light_point_sources[0].color = {1, 1, 1, 0};

	this->program_triangle_texture_uniforms.projection_matrix = this->program_triangle_color_uniforms.projection_matrix;
	this->program_triangle_texture_uniforms.ambient_light_color = this->program_triangle_color_uniforms.ambient_light_color;
#else
	this->projection_matrix = Mylib::Math::gen_identity_matrix<fp_t, 4>();
#endif

#if 0
	dprintln("projection matrix:");
	dprintln(this->projection_matrix);
	dprintln();
#endif
}

// ---------------------------------------------------

void Renderer::render ()
{
	this->program_triangle_color_uniforms.point_light_pos = this->light_point_sources[0].pos;
	this->program_triangle_color_uniforms.point_light_color = this->light_point_sources[0].color;

	this->program_triangle_texture_uniforms.point_light_pos = this->program_triangle_color_uniforms.point_light_pos;
	this->program_triangle_texture_uniforms.point_light_color = this->program_triangle_color_uniforms.point_light_color;
	
	this->program_triangle_color->load();
	this->program_triangle_color->upload_uniforms(this->program_triangle_color_uniforms);
	this->program_triangle_color->upload_vertex_buffer();
	this->program_triangle_color->draw();

	this->program_triangle_texture->load();
	this->program_triangle_texture->upload_uniforms(this->program_triangle_texture_uniforms);
	this->program_triangle_texture->upload_vertex_buffer();
	this->program_triangle_texture->draw();
}

// ---------------------------------------------------

void Renderer::update_screen ()
{
	SDL_GL_SwapWindow(this->sdl_window);
}

// ---------------------------------------------------

void Renderer::clear_vertex_buffers ()
{
	this->program_triangle_color->clear();
	this->program_triangle_texture->clear();
}

// ---------------------------------------------------

void Renderer::begin_texture_loading ()
{

}

// ---------------------------------------------------

void Renderer::end_texture_loading ()
{

}

// ---------------------------------------------------

TextureDescriptor Renderer::load_texture (SDL_Surface *surface)
{

}

// ---------------------------------------------------

void Renderer::destroy_texture (TextureDescriptor& texture)
{

}

// ---------------------------------------------------

} // namespace Graphics
} // namespace Opengl
} // namespace MyGlib