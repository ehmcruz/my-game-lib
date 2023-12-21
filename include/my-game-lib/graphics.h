#ifndef __MY_GAME_LIB_GRAPHICS_HEADER_H__
#define __MY_GAME_LIB_GRAPHICS_HEADER_H__

#ifdef __MINGW32__
	#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>

#include <cstring>
#include <cmath>

#include <string>
#include <algorithm>
#include <array>
#include <string_view>
#include <span>
#include <string_view>

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/any.h>
#include <my-lib/memory.h>
#include <my-lib/math.h>
#include <my-lib/math-matrix.h>
#include <my-lib/math-geometry.h>

#include <my-game-lib/debug.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

#ifndef MYGLIB_FP_TYPE
	#error "MYGLIB_FP_TYPE must be defined"
#endif

#ifndef MYGLIB_SUPPORT_SDL
	#error "SDL support is required"
#endif

using fp_t = MYGLIB_FP_TYPE;
using Vector2 = Mylib::Math::Vector<fp_t, 2>;
using Vector3 = Mylib::Math::Vector<fp_t, 3>;
using Vector4 = Mylib::Math::Vector<fp_t, 4>;
using Line = Mylib::Math::Line<fp_t, 3>;
using Matrix3 = Mylib::Math::Matrix<fp_t, 3, 3>;
using Matrix4 = Mylib::Math::Matrix<fp_t, 4, 4>;
using Point2 = Vector2;
using Point3 = Vector3;
using Point4 = Vector4;
using Vector = Vector3;
using Point = Vector;

using Vector2f = Mylib::Math::Vector<float, 2>;
using Vector3f = Mylib::Math::Vector<float, 3>;
using Vector4f = Mylib::Math::Vector<float, 4>;

// ---------------------------------------------------

consteval fp_t fp (const auto v)
{
	return static_cast<fp_t>(v);
}

// ---------------------------------------------------

struct Color {
	float r;
	float g;
	float b;
	float a; // alpha

	float* get_raw () noexcept
	{
		return &this->r;
	}

	const float* get_raw () const noexcept
	{
		return &this->r;
	}

	static consteval Color black ()
	{
		return Color{0.0f, 0.0f, 0.0f, 1.0f};
	}

	static consteval Color white ()
	{
		return Color{1.0f, 1.0f, 1.0f, 1.0f};
	}

	static consteval Color red ()
	{
		return Color{1.0f, 0.0f, 0.0f, 1.0f};
	}

	static consteval Color green ()
	{
		return Color{0.0f, 1.0f, 0.0f, 1.0f};
	}

	static consteval Color blue ()
	{
		return Color{0.0f, 0.0f, 1.0f, 1.0f};
	}
};

std::ostream& operator << (std::ostream& out, const Color& color);

// ---------------------------------------------------

struct TextureDescriptor {
	Mylib::Any<sizeof(void*), sizeof(void*)> data; // used by backend driver
	uint32_t width_px;
	uint32_t height_px;
};

// ---------------------------------------------------

struct Vertex {
	Point pos; // local x,y,z coords
	Vector normal; // normal vector used for lighting
};

// ---------------------------------------------------

class Shape
{
public:
	enum class Type {
		Rect2D,
		Circle2D,
		Cube3D,
		Sphere3D,
		Undefined // very usefull for library users
	};

protected:
	OO_ENCAPSULATE_SCALAR_READONLY(Type, type)

	OO_ENCAPSULATE_SCALAR_INIT_READONLY(fp_t, rotation_angle, 0)
	OO_ENCAPSULATE_OBJ_INIT_READONLY(Vector, rotation_axis, Vector::zero())

private:
	std::span<Vertex> local_vertices_buffer__; // not rotated
	std::span<Vertex> local_rotated_vertices_buffer__;
	bool must_recalculate_rotation = false;

	void calculate_rotation ();

protected:
	Shape (const Type type_) noexcept
		: type(type_)
	{
	}

	void shape_copy (const Shape& other) noexcept
	{
		this->rotation_angle = other.rotation_angle;
		this->rotation_axis = other.rotation_axis;
	}

public:
	virtual ~Shape () = default;

	MYLIB_DELETE_COPY_MOVE_CONSTRUCTOR_ASSIGN(Shape)

	inline std::span<Vertex> get_local_rotated_vertices () noexcept
	{
		if (this->rotation_angle == fp(0))
			return this->local_vertices_buffer__;
		if (this->must_recalculate_rotation)
			this->calculate_rotation();
		return this->local_rotated_vertices_buffer__;
	}

	constexpr void rotate (const Vector& axis, const fp_t angle) noexcept
	{
		this->rotation_axis = axis;
		this->rotation_angle = std::fmod(angle, Mylib::Math::degrees_to_radians(fp(360)));
		this->must_recalculate_rotation = true;
	}

	constexpr void rotate (const fp_t angle) noexcept
	{
		this->rotation_angle = std::fmod(angle, Mylib::Math::degrees_to_radians(fp(360)));
		this->must_recalculate_rotation = true;
	}

protected:
	inline void set_vertices_buffer (const std::span<Vertex> local_vertices_buffer,
	                                 const std::span<Vertex> local_rotated_vertices_buffer
	                                 ) noexcept
	{
		this->local_vertices_buffer__ = local_vertices_buffer;
		this->local_rotated_vertices_buffer__ = local_rotated_vertices_buffer;
	}

	inline void force_recalculate_rotation () noexcept
	{
		this->must_recalculate_rotation = true;
	}
};

// ---------------------------------------------------

class Cube3D : public Shape
{
public:
	static consteval uint32_t get_n_vertices ()
	{
		return 36; // 6 sides * 2 triangles * 3 vertices
	}

	enum PositionIndex {
		LeftTopFront,
		LeftBottomFront,
		RightTopFront,
		RightBottomFront,
		LeftTopBack,
		LeftBottomBack,
		RightTopBack,
		RightBottomBack
	};

protected:
	OO_ENCAPSULATE_SCALAR_REACT(fp_t, w, this->calculate_vertices();) // width
	OO_ENCAPSULATE_SCALAR_REACT(fp_t, h, this->calculate_vertices();) // height
	OO_ENCAPSULATE_SCALAR_REACT(fp_t, d, this->calculate_vertices();) // depth

private:
	std::array<Vertex, 36> vertices; // 6 sides * 2 triangles * 3 vertices
	std::array<Vertex, 36> rotated_vertices; // 6 sides * 2 triangles * 3 vertices

public:
	Cube3D (const fp_t w_) noexcept
		: Shape(Type::Cube3D), w(w_), h(w_), d(w_)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	Cube3D (const fp_t w_, const fp_t h_, const fp_t d_) noexcept
		: Shape(Type::Cube3D), w(w_), h(h_), d(d_)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	Cube3D () noexcept
		: Shape(Type::Cube3D)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
	}

	// copy constructor
	Cube3D (const Cube3D& other)
		: Shape(Type::Cube3D), w(other.w), h(other.h), d(other.d)
	{
		this->shape_copy(other);
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	// copy-assign operator
	Cube3D& operator= (const Cube3D& other)
	{
		this->type = Type::Cube3D;
		this->w = other.w;
		this->h = other.h;
		this->d = other.d;
		this->shape_copy(other);
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();

		return *this;
	}

	void set_size (const fp_t w, const fp_t h, const fp_t d) noexcept
	{
		this->w = w;
		this->h = h;
		this->d = d;
		this->calculate_vertices();
	}

	void set_size (const fp_t w) noexcept
	{
		this->w = w;
		this->h = w;
		this->d = w;
		this->calculate_vertices();
	}

	void calculate_vertices () noexcept;
};

// ---------------------------------------------------

class Sphere3D : public Shape
{
protected:
	OO_ENCAPSULATE_SCALAR_REACT(fp_t, radius, this->calculate_vertices();)

private:
	std::vector<Vertex> vertices;

	/*
		We rotate Spheres3D in a shader, since rotating a sphere doesn't
		change its vertices.
		Why rotate a sphere then? Because of the textures.
	*/

public:
	Sphere3D (const fp_t radius_)
		: Shape (Type::Sphere3D),
		  radius(radius_)
	{
		this->calculate_vertices();
	}

	Sphere3D ()
		: Shape (Type::Sphere3D)
	{
	}

	// copy constructor
	Sphere3D (const Sphere3D& other)
		: Shape(Type::Sphere3D), radius(other.radius)
	{
		this->shape_copy(other);
		this->calculate_vertices();
	}

	// copy-assign operator
	Sphere3D& operator= (const Sphere3D& other)
	{
		this->type = Type::Sphere3D;
		this->radius = other.radius;
		this->shape_copy(other);
		this->calculate_vertices();

		return *this;
	}

	void setup_vertices_buffer (const uint32_t n_vertices);
	void calculate_vertices ();

	inline uint32_t get_n_vertices () const noexcept
	{
		return this->vertices.size();
	}
};

// ---------------------------------------------------

class CircleFactory;

class Circle2D : public Shape
{
protected:
	OO_ENCAPSULATE_SCALAR_REACT(fp_t, radius, this->calculate_vertices();)

private:
	std::vector<Vertex> vertices;
	std::vector<Vertex> rotated_vertices;

	/*
		As with Spheres3D, rotating a Circle2D doesn't change its vertices positions.
		Also as well, we nvertheless need to rotate them because of the textures.
		However, we are going to rotate Circle2D in the CPU, since the
		performance impact of a Circle2D is low.
	*/

public:
	Circle2D (const fp_t radius_)
		: Shape (Type::Circle2D),
		  radius(radius_)
	{
		this->calculate_vertices();
	}

	Circle2D ()
		: Shape (Type::Circle2D)
	{
	}

	void setup_vertices_buffer (const uint32_t n_vertices);
	void calculate_vertices (const CircleFactory& factory);
	void calculate_vertices (const Matrix4& projection_matrix);
	void calculate_vertices ();

	inline uint32_t get_n_vertices () const noexcept
	{
		return this->vertices.size();
	}
};

// ---------------------------------------------------

class Rect2D : public Shape
{
public:
	static consteval uint32_t get_n_vertices ()
	{
		return 6; // 2 triangles
	}

protected:
	OO_ENCAPSULATE_SCALAR_REACT(fp_t, w, this->calculate_vertices();)
	OO_ENCAPSULATE_SCALAR_REACT(fp_t, h, this->calculate_vertices();)
	//OO_ENCAPSULATE_SCALAR_INIT(fp_t, z, 0)

private:
	std::array<Vertex, 6> vertices; // 2 triangles
	std::array<Vertex, 6> rotated_vertices; // 2 triangles

public:
	Rect2D (const fp_t w_, const fp_t h_) noexcept
		: Shape (Type::Rect2D),
		  w(w_), h(h_)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	Rect2D () noexcept
		: Shape (Type::Rect2D)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
	}

	void set_size (const fp_t w, const fp_t h) noexcept
	{
		this->w = w;
		this->h = h;
		this->calculate_vertices();
	}

	void calculate_vertices () noexcept;
};

// ---------------------------------------------------

struct RenderArgs3D {
	Point world_camera_pos;
	Point world_camera_target;
	fp_t fov_y;
	fp_t z_near;
	fp_t z_far;
	Color ambient_light_color;
};

struct RenderArgs2D {
	/* Normalized clip area.
	   Values must be between 0.0f and 1.0f,
	   where 1.0f represents max(clip_width, clip_height).
	*/
	Vector2 clip_init_norm;
	Vector2 clip_end_norm;

	/* The minimum and maximum possible coordinates of the world.
	*/
	Vector2 world_init;
	Vector2 world_end;

	bool force_camera_inside_world;
	Vector2 world_camera_focus;

	/* This is the width in world coords that will fit in the clip_spice.
	   The lower the value, the highest the zoom.
	   This should NEVER be higher then (world_end.x - world_init.x).
	   In case it is, it will be automatically set to (world_end.x - world_init.x).
	*/
	fp_t world_screen_width;
	// world_screen_height will be calculated automatically from the aspect ratio
};

// ---------------------------------------------------

using LightPointDescriptor = uint32_t;

// ---------------------------------------------------

class Manager
{
protected:
	static inline constexpr uint32_t max_points_light_source = 5;

public:
	enum class Type : uint32_t { // any change here will need a change in get_type_str
	#ifdef MYGLIB_SUPPORT_SDL
		SDL,
	#endif
	#ifdef MYGLIB_SUPPORT_OPENGL
		Opengl,
	#endif
	#ifdef MYGLIB_SUPPORT_VULKAN
		Vulkan,
	#endif
		Unsupported // must be the last one
	};

	static const char* get_type_str (const Type value);

	struct InitParams {
		Mylib::Memory::Manager& memory_manager;
		std::string_view window_name;
		uint32_t window_width_px;
		uint32_t window_height_px;
		bool fullscreen;
	};

protected:
	Mylib::Memory::Manager& memory_manager;
	OO_ENCAPSULATE_SCALAR_READONLY(uint32_t, window_width_px)
	OO_ENCAPSULATE_SCALAR_READONLY(uint32_t, window_height_px)
	OO_ENCAPSULATE_SCALAR_READONLY(bool, fullscreen)
	
	OO_ENCAPSULATE_OBJ_INIT(Color, background_color, Color::black())

	struct LightPointSource {
		Point pos;
		Color color;
		bool busy = false;
	};

	std::array<LightPointSource, max_points_light_source> light_point_sources;

	SDL_Window *sdl_window;

public:
	Manager (const InitParams& params)
		: memory_manager(params.memory_manager),
		window_width_px(params.window_width_px),
		window_height_px(params.window_height_px),
		fullscreen(params.fullscreen)
	{
	}

	virtual ~Manager () = default;

	inline fp_t get_window_aspect_ratio () const
	{
		return static_cast<fp_t>(this->window_width_px) / static_cast<fp_t>(this->window_height_px);
	}

	inline fp_t get_inverted_window_aspect_ratio () const
	{
		return fp(1) / this->get_window_aspect_ratio();
	}

	inline Vector2 get_normalized_window_size () const
	{
		const fp_t max_value = static_cast<fp_t>( std::max(this->window_width_px, this->window_height_px) );
		return Vector2(static_cast<fp_t>(this->window_width_px) / max_value, static_cast<fp_t>(this->window_height_px) / max_value);
	}

	/*
		We don't receive the shapes as const in the draw functions
		because we need to manipulate their buffers.
	*/

	virtual void wait_next_frame () = 0;
	virtual void draw_cube3D (Cube3D& cube, const Vector& offset, const Color& color) = 0;
	virtual void draw_sphere3D (Sphere3D& sphere, const Vector& offset, const Color& color) = 0;
	virtual void draw_circle2D (Circle2D& circle, const Vector& offset, const Color& color) = 0;
	virtual void draw_rect2D (Rect2D& rect, const Vector& offset, const Color& color) = 0;
	virtual void draw_rect2D (Rect2D& rect, const Vector& offset, const TextureDescriptor& texture_desc) = 0;
	virtual void setup_render_3D (const RenderArgs3D& args) = 0;
	virtual void setup_render_2D (const RenderArgs2D& args) = 0;
	virtual void render () = 0;
	virtual void update_screen () = 0;
	virtual void clear_vertex_buffers () = 0;
	virtual TextureDescriptor load_texture (SDL_Surface *surface) = 0;
	virtual void destroy_texture (TextureDescriptor& texture) = 0;

	// 2D Wrappers

	void draw_rect2D (Rect2D&& rect, const Vector& offset, const Color& color)
	{
		this->draw_rect2D(rect, offset, color);
	}

	void draw_circle2D (Circle2D&& circle, const Vector& offset, const Color& color)
	{
		this->draw_circle2D(circle, offset, color);
	}

	void draw_circle2D (Circle2D& circle, const Vector2& offset, const Color& color)
	{
		this->draw_circle2D(circle, Vector(offset.x, offset.y, 0), color);
	}
	
	void draw_rect2D (Rect2D& rect, const Vector2& offset, const Color& color)
	{
		this->draw_rect2D(rect, Vector(offset.x, offset.y, 0), color);
	}

	// Texture wrappers

	TextureDescriptor load_texture (const std::string_view fname);

	// light functions

	[[nodiscard]] LightPointDescriptor add_light_point_source (const Point& pos, const Color& color);
	
	inline void move_light_point_source (const LightPointDescriptor desc, const Point& pos)
	{
		LightPointSource& light_source = this->light_point_sources[desc];
		light_source.pos = pos;
	}
};

// ---------------------------------------------------

class CircleFactory
{
private:
	std::vector<fp_t> table_sin;
	std::vector<fp_t> table_cos;
	const uint32_t n_triangles;

public:
	CircleFactory (const uint32_t n_triangles_);

	inline uint32_t get_n_vertices () const noexcept
	{
		return (this->n_triangles * 3);
	}

	void build_circle (const fp_t radius, std::span<Vertex> vertices) const;
};

// ---------------------------------------------------

class CircleFactoryManager
{
private:
	std::vector<CircleFactory> factories;
public:
	CircleFactoryManager (const uint32_t n_cats, const uint32_t min_n_triangles, const uint32_t max_n_triangles);

	inline const CircleFactory& get_factory (fp_t screen_size_per_cent) const noexcept
	{
		if (screen_size_per_cent > fp(1))
			screen_size_per_cent = fp(1); // max value
		uint32_t index = static_cast<uint32_t>(factories.size() * screen_size_per_cent);
		if (index >= factories.size()) [[unlikely]]
			index = factories.size() - 1;
		return this->factories[index];
	}
};

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib

#endif