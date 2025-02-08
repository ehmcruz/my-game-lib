#include <iostream>
#include <fstream>
#include <sstream>
#include <numbers>
#include <utility>

#include <cstdlib>
#include <cmath>

#include <SDL_image.h>

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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	this->program_triangle_texture_rotation = new ProgramTriangleTextureRotation;

	dprintln("all opengl programs loaded");
}

// ---------------------------------------------------

Renderer::~Renderer ()
{
	delete this->program_triangle_color;
	delete this->program_triangle_texture;
	delete this->program_triangle_texture_rotation;

	SDL_GL_DeleteContext(this->sdl_gl_context);
	SDL_DestroyWindow(this->sdl_window);
}

// ---------------------------------------------------

void Renderer::wait_next_frame ()
{
	this->clear_buffers(ColorBufferBit | DepthBufferBit | VertexBufferBit);
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

void Renderer::draw_cube3D (Cube3D& cube, const Vector& offset, const std::array<TextureRenderOptions, 6>& texture_options)
{
	constexpr uint32_t n_vertices = Cube3D::get_n_vertices();
	std::span<ProgramTriangleTexture::Vertex> vertices = this->program_triangle_texture->alloc_vertices(n_vertices);
	std::span<Vertex> shape_vertices = cube.get_local_rotated_vertices();

	mylib_assert_exception(shape_vertices.size() == n_vertices)

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex = shape_vertices[i];
		vertices[i].offset = offset;
	}

	// Texture coordinates must be applied in the same order
	// as the vertices are calculated in Cube3D::calculate_vertices

	uint32_t i = 0;

	using VertexPositionIndex = Cube3D::VertexPositionIndex;
	using enum Cube3D::VertexPositionIndex;
	using enum Cube3D::SurfacePositionIndex;

	auto mount = [&i, vertices] (const VertexPositionIndex p, const Vector3f& v, const TextureRenderOptions& texture_options) -> void {
		const Opengl_TextureDescriptor *desc = texture_options.desc.info->data.get_value<Opengl_TextureDescriptor*>();
		const Opengl_AtlasDescriptor *atlas = desc->atlas;

		vertices[i].tex_coords = Vector3f(v.x, v.y, atlas->texture_depth);
		i++;
	};

	auto mount_triangle = [&mount] (const VertexPositionIndex p1, const VertexPositionIndex p2, const VertexPositionIndex p3, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, const TextureRenderOptions& texture_options) -> void {
		mount(p1, v1, texture_options);
		mount(p2, v2, texture_options);
		mount(p3, v3, texture_options);
	};

	// p1 and p2 should be a diagonal of the rectangle
	auto mount_surface = [&mount_triangle] (const VertexPositionIndex p1, const VertexPositionIndex p2, const VertexPositionIndex p3, const VertexPositionIndex p4, const TextureRenderOptions& texture_options) -> void {
		const Opengl_TextureDescriptor *desc = texture_options.desc.info->data.get_value<Opengl_TextureDescriptor*>();

		mount_triangle(p1, p2, p3, desc->tex_coords[Rect2D::LeftTop], desc->tex_coords[Rect2D::RightBottom], desc->tex_coords[Rect2D::RightTop], texture_options);
		mount_triangle(p1, p2, p4, desc->tex_coords[Rect2D::LeftTop], desc->tex_coords[Rect2D::RightBottom], desc->tex_coords[Rect2D::LeftBottom], texture_options);
	};

	// bottom
	mount_surface(LeftBottomFront, RightBottomBack, RightBottomFront, LeftBottomBack, texture_options[Bottom]);

	// top
	mount_surface(LeftTopFront, RightTopBack, RightTopFront, LeftTopBack, texture_options[Top]);

	// front
	mount_surface(LeftTopFront, RightBottomFront, RightTopFront, LeftBottomFront, texture_options[Front]);

	// back
	mount_surface(LeftTopBack, RightBottomBack, RightTopBack, LeftBottomBack, texture_options[Back]);

	// left
	mount_surface(LeftTopFront, LeftBottomBack, LeftTopBack, LeftBottomFront, texture_options[Left]);

	// right
	mount_surface(RightTopFront, RightBottomBack, RightTopBack, RightBottomFront, texture_options[Right]);
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

void Renderer::draw_sphere3D (Sphere3D& sphere, const Vector& offset, const TextureRenderOptions& texture_options)
{
	const uint32_t n_vertices = sphere.get_n_vertices();
	std::span<Vertex> shape_vertices = sphere.get_local_vertices();

	mylib_assert_exception(shape_vertices.size() == n_vertices)

	const Opengl_TextureDescriptor *desc = texture_options.desc.info->data.get_value<Opengl_TextureDescriptor*>();
	const Opengl_AtlasDescriptor *atlas = desc->atlas;

	auto fill_vertices = [&sphere, &offset, &texture_options, n_vertices, shape_vertices, desc, atlas] (auto& program) -> void {
		auto vertices = program.alloc_vertices(n_vertices);

		for (uint32_t i=0; i<n_vertices; i++) {
			vertices[i].gvertex = shape_vertices[i];
			vertices[i].offset = offset;
		}

		// we have to follow the same order used in Sphere3D::calculate_vertices

		using enum Rect2D::VertexPositionIndex;

		const uint32_t u_resolution = sphere.get_u_resolution(); // longitude
		const uint32_t v_resolution = sphere.get_v_resolution(); // latitude

		const fp_t start_u = desc->tex_coords[LeftTop].x;
		const fp_t start_v = desc->tex_coords[LeftTop].y;

		const fp_t end_u = desc->tex_coords[RightBottom].x;
		const fp_t end_v = desc->tex_coords[RightBottom].y;

		const fp_t step_u = (end_u - start_u) / static_cast<fp_t>(u_resolution);
		const fp_t step_v = (end_v - start_v) / static_cast<fp_t>(v_resolution);

		uint32_t k = 0;

		for (uint32_t i = 0; i < u_resolution; i++) {
			const fp_t u = static_cast<fp_t>(i) * step_u + start_u;
			const fp_t un = u + step_u;

			for (uint32_t j = 0; j < v_resolution; j++) {
				const fp_t v = static_cast<fp_t>(j) * step_v + start_v;
				const fp_t vn = v + step_v;

				const Point3f p0 = Point3f(u, v, atlas->texture_depth);
				const Point3f p1 = Point3f(u, vn, atlas->texture_depth);
				const Point3f p2 = Point3f(un, v, atlas->texture_depth);
				const Point3f p3 = Point3f(un, vn, atlas->texture_depth);

				// Output the first triangle of this grid square
				vertices[k].tex_coords = p0;
				vertices[k + 1].tex_coords = p2;
				vertices[k + 2].tex_coords = p1;

				// Output the other triangle of this grid square
				vertices[k + 3].tex_coords = p3;
				vertices[k + 4].tex_coords = p1;
				vertices[k + 5].tex_coords = p2;

				k += 6;
			}
		}

		if constexpr (std::is_same_v<decltype(program), ProgramTriangleTextureRotation&>) {
			const Quaternion quaternion = Quaternion::rotation(sphere.get_ref_rotation_axis(), sphere.get_rotation_angle());

			for (uint32_t i=0; i<n_vertices; i++)
				vertices[i].rot_quat = quaternion;
		}
	};

	if (sphere.get_rotation_angle() == fp(0))
		fill_vertices(*this->program_triangle_texture);
	else
		fill_vertices(*this->program_triangle_texture_rotation);
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

void Renderer::draw_rect2D (Rect2D& rect, const Vector& offset, const TextureRenderOptions& texture_options)
{
	const Opengl_TextureDescriptor *desc = texture_options.desc.info->data.get_value<Opengl_TextureDescriptor*>();
	const Opengl_AtlasDescriptor *atlas = desc->atlas;

	constexpr uint32_t n_vertices = Rect2D::get_n_vertices();
	std::span<ProgramTriangleTexture::Vertex> vertices = this->program_triangle_texture->alloc_vertices(n_vertices);
	std::span<Vertex> shape_vertices = rect.get_local_rotated_vertices();

	static_assert(n_vertices == 6);
	mylib_assert_exception(shape_vertices.size() == n_vertices)

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex = shape_vertices[i];
		vertices[i].offset = offset;
	}

	// we have to follow the same order used in Rect2D::calculate_vertices

	using enum Rect2D::VertexPositionIndex;

	vertices[0].tex_coords = Vector3f(desc->tex_coords[LeftTop].x, desc->tex_coords[LeftTop].y, atlas->texture_depth); // upper left
	vertices[1].tex_coords = Vector3f(desc->tex_coords[RightBottom].x, desc->tex_coords[RightBottom].y, atlas->texture_depth); // down right
	vertices[2].tex_coords = Vector3f(desc->tex_coords[LeftBottom].x, desc->tex_coords[LeftBottom].y, atlas->texture_depth); // down left
	vertices[3].tex_coords = Vector3f(desc->tex_coords[LeftTop].x, desc->tex_coords[LeftTop].y, atlas->texture_depth); // upper left
	vertices[4].tex_coords = Vector3f(desc->tex_coords[RightTop].x, desc->tex_coords[RightTop].y, atlas->texture_depth); // upper right
	vertices[5].tex_coords = Vector3f(desc->tex_coords[RightBottom].x, desc->tex_coords[RightBottom].y, atlas->texture_depth); // down right
}

// ---------------------------------------------------

void Renderer::setup_render_3D (const RenderArgs3D& args)
{
	this->program_triangle_color_uniforms.projection_matrix = Matrix4::perspective(
			args.fov_y,
			static_cast<fp_t>(this->window_width_px),
			static_cast<fp_t>(this->window_height_px),
			args.z_near,
			args.z_far,
			fp(1)
		)
		* Matrix4::look_at(
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
	const Matrix4 translate_subtract_one = Matrix4::translate( Vector2(-1, +1) );
//	dprintln( "translation to clip init:" ) translate_to_clip_init.println();

	// mirror y axis
	// and also scale to (0, 2) coords
	const Matrix4 opengl_scale_mirror = Matrix4::scale(opengl_clip_scale_mirror);
//	dprintln( "scale matrix:" ) Mylib::Math::println(scale);
//exit(1);

	const Matrix4 translate_to_normalized_clip_init = Matrix4::translate(normalized_clip_init);
//	dprintln( "translation to clip init:" ) translate_to_clip_init.println();

	const Matrix4 scale_normalized = Matrix4::scale(Vector2(normalized_scale_factor, normalized_scale_factor));
//	dprintln( "scale matrix:" ) Mylib::Math::println(scale);
//exit(1);

	const Matrix4 translate_camera = Matrix4::translate(-world_camera);
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
	
	if (this->program_triangle_color->has_vertices()) {
		this->program_triangle_color->load();
		this->program_triangle_color->upload_uniforms(this->program_triangle_color_uniforms);
		this->program_triangle_color->upload_vertex_buffers();
		this->program_triangle_color->draw();
	}

	if (this->program_triangle_texture->has_vertices()) {
		this->program_triangle_texture->load();
		this->program_triangle_texture->upload_uniforms(this->program_triangle_texture_uniforms);
		this->program_triangle_texture->upload_vertex_buffers();
		this->program_triangle_texture->draw();
	}

	if (this->program_triangle_texture_rotation->has_vertices()) {
		this->program_triangle_texture_rotation->load();
		this->program_triangle_texture_rotation->upload_uniforms(this->program_triangle_texture_uniforms);
		this->program_triangle_texture_rotation->upload_vertex_buffers();
		this->program_triangle_texture_rotation->draw();
	}
}

// ---------------------------------------------------

void Renderer::update_screen ()
{
	SDL_GL_SwapWindow(this->sdl_window);
}

// ---------------------------------------------------

void Renderer::clear_buffers (const uint32_t flags)
{
	GLbitfield mask = 0;

	if (flags & VertexBufferBit) {
		this->program_triangle_color->clear();
		this->program_triangle_texture->clear();
		this->program_triangle_texture_rotation->clear();
	}

	if (flags & ColorBufferBit)
		mask |= GL_COLOR_BUFFER_BIT;
	if (flags & DepthBufferBit)
		mask |= GL_DEPTH_BUFFER_BIT;
	if (flags & StencilBufferBit)
		mask |= GL_STENCIL_BUFFER_BIT;

	if (mask)
		glClear(mask);
}

// ---------------------------------------------------

void Renderer::begin_texture_loading ()
{
	mylib_assert_exception(this->textures.empty())
}

// ---------------------------------------------------

void Renderer::end_texture_loading ()
{
	TextureAtlasCreator atlas_creator;

	for (auto& pair : this->textures) {
		TextureInfo& tex_desc = pair.second;
		atlas_creator.add_texture(tex_desc);
	}

	std::list< std::vector<TextureAtlasCreator::AtlasTexture> > atlas_list;

	while (true) {
		std::vector<TextureAtlasCreator::AtlasTexture> atlas = atlas_creator.create_atlas(max_texture_size);

		if (atlas.empty())
			break;
		
		atlas_list.push_back(std::move(atlas));
	}

	glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
	ensure_no_error();

	glGenTextures(1, &this->texture_array_id);
	ensure_no_error();

	glBindTexture(GL_TEXTURE_2D_ARRAY, this->texture_array_id);
	ensure_no_error();

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, max_texture_size, max_texture_size, atlas_list.size());
	ensure_no_error();

	for (GLint tex_depth = 0; auto& atlas : atlas_list) {
		this->atlases.push_back( Opengl_AtlasDescriptor {
			.texture_depth = static_cast<float>(tex_depth),
			.width_px = max_texture_size,
			.height_px = max_texture_size
		} );

		Opengl_AtlasDescriptor& atlas_desc = this->atlases.back();

#if 0
		constexpr int32_t bits = 32;
		constexpr Uint32 rmask = 0x000000FF;
		constexpr Uint32 gmask = 0x0000FF00;
		constexpr Uint32 bmask = 0x00FF0000;
		constexpr Uint32 amask = 0xFF000000;

		SDL_Surface *atlas_surface = SDL_CreateRGBSurface(0, max_texture_size, max_texture_size, bits, rmask, gmask, bmask, amask);
		mylib_assert_exception_msg(atlas_surface != nullptr, "error creating surface", '\n', SDL_GetError())
#endif

		dprintln("Atlas created with ", atlas.size(), " textures");

		for (auto& atlas_tex_desc : atlas) {
			TextureInfo& tex_desc = *atlas_tex_desc.texture;
			Opengl_TextureDescriptor *desc = tex_desc.data.get_value<Opengl_TextureDescriptor*>();

			dprintln("\tTexture of size ", tex_desc.width_px, "x", tex_desc.height_px, " allocated at position ", atlas_tex_desc.x_ini, "x", atlas_tex_desc.y_ini);

#if 0
			SDL_Rect rect = {
				.x = atlas_tex_desc.x_ini,
				.y = atlas_tex_desc.y_ini,
				.w = tex_desc.width_px,
				.h = tex_desc.height_px
			};

			SDL_BlitSurface(desc->surface, nullptr, atlas_surface, &rect);
#else
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
				0,
				atlas_tex_desc.x_ini,
				atlas_tex_desc.y_ini,
				tex_depth,
				tex_desc.width_px,
				tex_desc.height_px,
				1,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				desc->surface->pixels);
			
			ensure_no_error();
#endif
			SDL_FreeSurface(desc->surface);
			desc->surface = nullptr;

			desc->atlas = &atlas_desc;

			desc->x_init_px = atlas_tex_desc.x_ini;
			desc->y_init_px = atlas_tex_desc.y_ini;

			using enum Rect2D::VertexPositionIndex;

			desc->tex_coords[LeftTop] = Vector2f(static_cast<fp_t>(desc->x_init_px) / static_cast<fp_t>(max_texture_size), static_cast<fp_t>(desc->y_init_px) / static_cast<fp_t>(max_texture_size));
			desc->tex_coords[LeftBottom] = Vector2f(static_cast<fp_t>(desc->x_init_px) / static_cast<fp_t>(max_texture_size), static_cast<fp_t>(desc->y_init_px + desc->height_px) / static_cast<fp_t>(max_texture_size));
			desc->tex_coords[RightTop] = Vector2f(static_cast<fp_t>(desc->x_init_px + desc->width_px) / static_cast<fp_t>(max_texture_size), static_cast<fp_t>(desc->y_init_px) / static_cast<fp_t>(max_texture_size));
			desc->tex_coords[RightBottom] = Vector2f(static_cast<fp_t>(desc->x_init_px + desc->width_px) / static_cast<fp_t>(max_texture_size), static_cast<fp_t>(desc->y_init_px + desc->height_px) / static_cast<fp_t>(max_texture_size));
		}

#if 0
		{ static int i = 0; std::string fname = "atlas" + std::to_string(i++) + ".png"; IMG_SavePNG(atlas_surface, fname.data()); }

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
		                0,
						0,
						0,
						tex_depth,
						atlas_surface->w,
						atlas_surface->h,
						1,
						GL_RGBA,
						GL_UNSIGNED_BYTE,
						atlas_surface->pixels);

		ensure_no_error();

		SDL_FreeSurface(atlas_surface);
#endif

		tex_depth++;
	};
}

// ---------------------------------------------------

TextureInfo Renderer::load_texture__ (SDL_Surface *surface)
{
	Opengl_TextureDescriptor *desc = new(this->memory_manager.allocate_type<Opengl_TextureDescriptor>(1)) Opengl_TextureDescriptor;

	SDL_Surface *treated_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
	mylib_assert_exception_msg(treated_surface != nullptr, "error converting surface format", '\n', SDL_GetError())

	desc->surface = treated_surface;
	desc->atlas = nullptr;
	desc->width_px = treated_surface->w;
	desc->height_px = treated_surface->h;
	
/*	glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
	ensure_no_error();

	glGenTextures(1, &desc->texture_id);
	ensure_no_error();

	glBindTexture(GL_TEXTURE_2D, desc->texture_id);
	ensure_no_error();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	ensure_no_error();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, desc->width_px, desc->height_px, 0, GL_RGBA, GL_UNSIGNED_BYTE, treated_surface->pixels);
	ensure_no_error();

	glGenerateMipmap(GL_TEXTURE_2D);
	ensure_no_error();

	//SDL_FreeSurface(treated_surface);*/

	TextureInfo tex_info = {
		.data = desc,
		.width_px = desc->width_px,
		.height_px = desc->height_px,
		.aspect_ratio = static_cast<fp_t>(desc->width_px) / static_cast<fp_t>(desc->height_px)
		};
		
	return tex_info;
}

// ---------------------------------------------------

void Renderer::destroy_texture__ (TextureInfo& texture)
{
	mylib_throw_exception_msg("OpenGl Renderer does not support texture destruction");
}

// ---------------------------------------------------

TextureInfo Renderer::create_sub_texture__ (const TextureInfo& parent, const uint32_t x_ini, const uint32_t y_ini, const uint32_t w, const uint32_t h)
{
	Opengl_TextureDescriptor *desc = new(this->memory_manager.allocate_type<Opengl_TextureDescriptor>(1)) Opengl_TextureDescriptor;
	const Opengl_TextureDescriptor *parent_desc = parent.data.get_value<Opengl_TextureDescriptor*>();

	desc->surface = nullptr;
	desc->atlas = parent_desc->atlas;
	desc->x_init_px = parent_desc->x_init_px + x_ini;
	desc->y_init_px = parent_desc->y_init_px + y_ini;
	desc->width_px = w;
	desc->height_px = h;

	mylib_assert_exception(parent_desc->atlas != nullptr)
	mylib_assert_exception((desc->x_init_px + desc->width_px) <= parent_desc->atlas->width_px)
	mylib_assert_exception((desc->y_init_px + desc->height_px) <= parent_desc->atlas->height_px)

	using enum Rect2D::VertexPositionIndex;

	desc->tex_coords[LeftTop] = Vector2f(static_cast<fp_t>(desc->x_init_px) / static_cast<fp_t>(max_texture_size), static_cast<fp_t>(desc->y_init_px) / static_cast<fp_t>(max_texture_size));
	desc->tex_coords[LeftBottom] = Vector2f(static_cast<fp_t>(desc->x_init_px) / static_cast<fp_t>(max_texture_size), static_cast<fp_t>(desc->y_init_px + desc->height_px) / static_cast<fp_t>(max_texture_size));
	desc->tex_coords[RightTop] = Vector2f(static_cast<fp_t>(desc->x_init_px + desc->width_px) / static_cast<fp_t>(max_texture_size), static_cast<fp_t>(desc->y_init_px) / static_cast<fp_t>(max_texture_size));
	desc->tex_coords[RightBottom] = Vector2f(static_cast<fp_t>(desc->x_init_px + desc->width_px) / static_cast<fp_t>(max_texture_size), static_cast<fp_t>(desc->y_init_px + desc->height_px) / static_cast<fp_t>(max_texture_size));

	TextureInfo tex_info = {
		.data = desc,
		.width_px = desc->width_px,
		.height_px = desc->height_px,
		.aspect_ratio = static_cast<fp_t>(desc->width_px) / static_cast<fp_t>(desc->height_px)
		};
	
	return tex_info;
}

// ---------------------------------------------------

} // namespace Graphics
} // namespace Opengl
} // namespace MyGlib