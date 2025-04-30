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
#include <random>
#include <unordered_map>
#include <string>
#include <variant>

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/any.h>
#include <my-lib/memory.h>
#include <my-lib/math.h>
#include <my-lib/math-vector.h>
#include <my-lib/math-matrix.h>
#include <my-lib/math-geometry.h>
#include <my-lib/math-quaternion.h>
#include <my-lib/matrix.h>
#include <my-lib/utils.h>

#include <my-game-lib/debug.h>
#include <my-game-lib/exception.h>

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
using Quaternion = Mylib::Math::Quaternion<fp_t>;

using Vector2f = Mylib::Math::Vector<float, 2>;
using Vector3f = Mylib::Math::Vector<float, 3>;
using Vector4f = Mylib::Math::Vector<float, 4>;

using Point2f = Vector2f;
using Point3f = Vector3f;
using Point4f = Vector4f;

// ---------------------------------------------------

consteval fp_t fp (const auto v) noexcept
{
	return static_cast<fp_t>(v);
}

// ---------------------------------------------------

using Color = Vector4f;

// Using struct instead of namespace to allow importing
// using xxx = MyGlib::Graphics::Colors;

struct Colors
{
	static inline constexpr Color black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	static inline constexpr Color white = Color(1.0f, 1.0f, 1.0f, 1.0f);
	static inline constexpr Color red = Color(1.0f, 0.0f, 0.0f, 1.0f);
	static inline constexpr Color green = Color(0.0f, 1.0f, 0.0f, 1.0f);
	static inline constexpr Color blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
	static inline constexpr Color yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);

	static constexpr Color random (auto& gen)
	{
		std::uniform_real_distribution<float> dis(0.0f, 1.0f);
		return Color( dis(gen), dis(gen), dis(gen), 1.0f );
	}
};

// ---------------------------------------------------

struct TextureInfo {
	// filled by the backend driver
	Mylib::Any<sizeof(void*), sizeof(void*)> data;
	int32_t width_px;
	int32_t height_px;
	fp_t aspect_ratio;

	// filled by the frontend
	std::string id;
};

// ---------------------------------------------------

struct TextureDescriptor {
	TextureInfo *info;
};

// ---------------------------------------------------

struct Vertex {
	Point pos; // local x,y,z coords
	union {
		Vector normal; // normal vector used for lighting
		Vector direction; // line direction vector
	};

	Vertex () noexcept
		: pos(Point::zero()), normal(Vector::zero())
	{
	}
};

// ---------------------------------------------------

class Shape
{
public:
	enum class Type {
		Rect2D,
		Circle2D,
		Cube3D,
		WireCube3D,
		Sphere3D,
		Line3D,
		Undefined // may be useful
	};

protected:
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(Type, type)

	MYLIB_OO_ENCAPSULATE_SCALAR_INIT_READONLY(fp_t, rotation_angle, 0)
	MYLIB_OO_ENCAPSULATE_OBJ_INIT_READONLY(Vector, rotation_axis, Vector::zero())
	MYLIB_OO_ENCAPSULATE_OBJ_INIT_READONLY(Vector, scale, Vector(1, 1, 1))

private:
	// vertices are in local coords
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
		this->scale = other.scale;
		this->must_recalculate_rotation = true;
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

	inline std::span<Vertex> get_local_vertices () noexcept
	{
		return this->local_vertices_buffer__;
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

	// for 3D shapes
	inline void set_scale (const Vector& scale) noexcept
	{
		this->scale = scale;
		this->must_recalculate_rotation = true;
	}

	// for 2D shapes
	inline void set_scale (const Vector2 scale) noexcept
	{
		this->scale = Vector(scale.x, scale.y, 0);
		this->must_recalculate_rotation = true;
	}

	inline void set_scale_x (const fp_t scale) noexcept
	{
		this->scale.x = scale;
		this->must_recalculate_rotation = true;
	}

	inline void set_scale_y (const fp_t scale) noexcept
	{
		this->scale.y = scale;
		this->must_recalculate_rotation = true;
	}

	inline void set_scale_z (const fp_t scale) noexcept
	{
		this->scale.z = scale;
		this->must_recalculate_rotation = true;
	}

	inline void force_recalculate_rotation () noexcept
	{
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
};

// ---------------------------------------------------

class Cube3D : public Shape
{
public:
	static consteval uint32_t get_n_vertices () noexcept
	{
		return 36; // 6 sides * 2 triangles * 3 vertices
	}

	enum VertexPositionIndex {
		LeftTopFront,
		LeftBottomFront,
		RightTopFront,
		RightBottomFront,
		LeftTopBack,
		LeftBottomBack,
		RightTopBack,
		RightBottomBack
	};

	enum SurfacePositionIndex {
		Front,
		Back,
		Left,
		Right,
		Top,
		Bottom
	};

protected:
	// write functions of these 3 variables are written bellow the constructor
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, w) // width
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, h) // height
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, d) // depth

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

	inline void set_w (const fp_t w) noexcept
	{
		this->w = w;
		this->calculate_vertices();
	}

	inline void set_h (const fp_t h) noexcept
	{
		this->h = h;
		this->calculate_vertices();
	}

	inline void set_d (const fp_t d) noexcept
	{
		this->d = d;
		this->calculate_vertices();
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

class WireCube3D : public Shape
{
public:
	static consteval uint32_t get_n_vertices () noexcept
	{
		return 24; // 12 lines * 2 vertices_per_line
	}

	using enum Cube3D::VertexPositionIndex;

protected:
	// write functions of these 3 variables are written bellow the constructor
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, w) // width
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, h) // height
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, d) // depth

private:
	std::array<Vertex, 24> vertices;
	std::array<Vertex, 24> rotated_vertices;

public:
	WireCube3D (const fp_t w_) noexcept
		: Shape(Type::WireCube3D), w(w_), h(w_), d(w_)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	WireCube3D (const fp_t w_, const fp_t h_, const fp_t d_) noexcept
		: Shape(Type::WireCube3D), w(w_), h(h_), d(d_)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	WireCube3D (const Vector& size) noexcept
		: Shape(Type::WireCube3D), w(size.x), h(size.y), d(size.z)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	WireCube3D () noexcept
		: Shape(Type::WireCube3D)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
	}

	// copy constructor
	WireCube3D (const WireCube3D& other)
		: Shape(Type::WireCube3D), w(other.w), h(other.h), d(other.d)
	{
		this->shape_copy(other);
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	// copy-assign operator
	WireCube3D& operator= (const WireCube3D& other)
	{
		this->w = other.w;
		this->h = other.h;
		this->d = other.d;
		this->shape_copy(other);
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();

		return *this;
	}

	inline void set_w (const fp_t w) noexcept
	{
		this->w = w;
		this->calculate_vertices();
	}

	inline void set_h (const fp_t h) noexcept
	{
		this->h = h;
		this->calculate_vertices();
	}

	inline void set_d (const fp_t d) noexcept
	{
		this->d = d;
		this->calculate_vertices();
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
	// write functions of this variable is written bellow the constructor
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, radius)

	MYLIB_OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint32_t, u_resolution, 100) // longitude
	MYLIB_OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint32_t, v_resolution, 50) // latitude

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

	inline void set_radius (const fp_t radius)
	{
		this->radius = radius;
		this->calculate_vertices();
	}

	void setup_vertices_buffer (const uint32_t n_vertices);
	void calculate_vertices ();

	inline uint32_t get_n_vertices () const noexcept
	{
		return this->vertices.size();
	}

	void set_resolution (const uint32_t u_resolution, const uint32_t v_resolution) noexcept
	{
		this->u_resolution = u_resolution;
		this->v_resolution = v_resolution;
		this->calculate_vertices();
	}
};

// ---------------------------------------------------

class CircleFactory;

class Circle2D : public Shape
{
protected:
	// write functions of this variable is written bellow the constructor
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, radius)

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

	inline void set_radius (const fp_t radius)
	{
		this->radius = radius;
		this->calculate_vertices();
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
	enum VertexPositionIndex {
		LeftTop,
		LeftBottom,
		RightTop,
		RightBottom
	};

	static consteval uint32_t get_n_vertices ()
	{
		return 6; // 2 triangles
	}

protected:
	// write functions of these 2 variables are written bellow the constructor
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, w)
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(fp_t, h)
	//OO_ENCAPSULATE_SCALAR_INIT(fp_t, z, 0)

private:
	std::array<Vertex, 6> vertices; // 2 triangles
	std::array<Vertex, 6> rotated_vertices; // 2 triangles

public:
	// constructors

	Rect2D (const fp_t w_, const fp_t h_) noexcept
		: Shape (Type::Rect2D), w(w_), h(h_)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	Rect2D () noexcept
		: Shape (Type::Rect2D)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
	}

	Rect2D (const Rect2D& other)
		: Shape(Type::Rect2D), w(other.w), h(other.h)
	{
		this->shape_copy(other);
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->calculate_vertices();
	}

	// assignment operator

	Rect2D& operator= (const Rect2D& other)
	{
		//mylib_assert_exception(this->type == Type::Rect2D)
		this->w = other.w;
		this->h = other.h;
		this->shape_copy(other);
		this->calculate_vertices();

		return *this;
	}

	inline void set_w (const fp_t w) noexcept
	{
		this->w = w;
		this->calculate_vertices();
	}

	inline void set_h (const fp_t h) noexcept
	{
		this->h = h;
		this->calculate_vertices();
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

class Line3D : public Shape
{
public:
	static consteval uint32_t get_n_vertices () noexcept
	{
		return 2;
	}

private:
	std::array<Vertex, 2> vertices;
	std::array<Vertex, 2> rotated_vertices;

public:
	// constructors

	// This constructor assumes that the first vertex is in the origin
	Line3D (const Vector& direction) noexcept
		: Shape (Type::Line3D)
	{
		this->vertices[0].pos = Point::zero();
		this->vertices[0].direction = direction;
		this->vertices[1].pos = direction;
		this->vertices[1].direction = direction;
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
	}

	Line3D () noexcept
		: Shape (Type::Line3D)
	{
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
	}

	Line3D (const Line3D& other)
		: Shape(Type::Line3D)
	{
		this->shape_copy(other);
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
		this->vertices = other.vertices;
	}

	// assignment operator

	Line3D& operator= (const Line3D& other)
	{
		//mylib_assert_exception(this->type == Type::Line3D)
		this->shape_copy(other);
		this->vertices = other.vertices;

		return *this;
	}

	Line3D& operator= (const Vector& direction)
	{
		this->vertices[0].pos = Point::zero();
		this->vertices[0].direction = direction;
		this->vertices[1].pos = direction;
		this->vertices[1].direction = direction;
		this->force_recalculate_rotation();
		return *this;
	}
};

// ---------------------------------------------------

struct PerspectiveProjectionInfo {
	fp_t fov_y;
	fp_t z_near;
	fp_t z_far;
};

struct OrthogonalProjectionInfo {
	fp_t view_width; // the height will be calculated automatically from the aspect ratio
	fp_t z_near;
	fp_t z_far;
};

struct RenderArgs3D {
	Point world_camera_pos;
	Point world_camera_target;
	Vector world_camera_up;
	std::variant<PerspectiveProjectionInfo, OrthogonalProjectionInfo> projection;
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

struct TextureRenderOptions {
	TextureDescriptor desc;
//	bool flip_x = false;
//	bool flip_y = false;
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

	enum ClearFlags : uint32_t {
		ColorBufferBit        = 1,
		DepthBufferBit        = 2,
		StencilBufferBit      = 4,
		VertexBufferBit      = 8,
	};

protected:
	Mylib::Memory::Manager& memory_manager;
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(uint32_t, window_width_px)
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(uint32_t, window_height_px)
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(bool, fullscreen)
	
	MYLIB_OO_ENCAPSULATE_OBJ_INIT(Color, background_color, Colors::black)

	struct LightPointSource {
		Point pos;
		Color color;
		bool busy = false;
	};

	std::array<LightPointSource, max_points_light_source> light_point_sources;

	SDL_Window *sdl_window;

	Mylib::unordered_map_string_key<TextureInfo> textures;

private:
	uint64_t next_random_tex_id = 0;

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
	virtual void draw_line3D (Line3D& line, const Vector& offset, const Color& color) = 0;
	virtual void draw_cube3D (Cube3D& cube, const Vector& offset, const Color& color) = 0;
	virtual void draw_cube3D (Cube3D& cube, const Vector& offset, const std::array<TextureRenderOptions, 6>& texture_options) = 0;
	virtual void draw_wire_cube3D (WireCube3D& cube, const Vector& offset, const Color& color) = 0;
	virtual void draw_sphere3D (Sphere3D& sphere, const Vector& offset, const Color& color) = 0;
	virtual void draw_sphere3D (Sphere3D& sphere, const Vector& offset, const TextureRenderOptions& texture_options) = 0;
	virtual void draw_circle2D (Circle2D& circle, const Vector& offset, const Color& color) = 0;
	virtual void draw_rect2D (Rect2D& rect, const Vector& offset, const Color& color) = 0;
	virtual void draw_rect2D (Rect2D& rect, const Vector& offset, const TextureRenderOptions& texture_options) = 0;
	virtual void setup_render_3D (const RenderArgs3D& args) = 0;
	virtual void setup_render_2D (const RenderArgs2D& args) = 0;
	virtual void render () = 0;
	virtual void update_screen () = 0;
	virtual void clear_buffers (const uint32_t flags) = 0;

	virtual void begin_texture_loading () = 0;
	virtual void end_texture_loading () = 0;

	// 3D Wrappers

	void draw_line3D (Line3D&& line, const Vector& offset, const Color& color)
	{
		this->draw_line3D(line, offset, color);
	}

	void draw_cube3D (Cube3D&& cube, const Vector& offset, const Color& color)
	{
		this->draw_cube3D(cube, offset, color);
	}

	void draw_cube3D (Cube3D& cube, const Vector& offset, const TextureRenderOptions& texture_options)
	{
		const std::array<TextureRenderOptions, 6> texture_options_array = {
			texture_options,
			texture_options,
			texture_options,
			texture_options,
			texture_options,
			texture_options
		};

		this->draw_cube3D(cube, offset, texture_options_array);
	}

	void draw_wire_cube3D (WireCube3D&& cube, const Vector& offset, const Color& color)
	{
		this->draw_wire_cube3D(cube, offset, color);
	}

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

	void draw_rect2D (Rect2D&& rect, const Vector& offset, const TextureRenderOptions& texture_options)
	{
		this->draw_rect2D(rect, offset, texture_options);
	}

	// Texture wrappers

	// To create and destroy textures, we always use these wrappers,
	// because the frontend manages the textures in a map, to allow
	// searching by the id.
	// We don't need to worry about passing a TextureDescriptor to the
	// backend in the render functions, because they don't change the map.

	TextureDescriptor load_texture (std::string id, SDL_Surface *surface);
	TextureDescriptor load_texture (std::string id, const std::string_view fname);
	void destroy_texture (TextureDescriptor& texture__);
	TextureDescriptor create_sub_texture (std::string id, const TextureDescriptor& parent__, const uint32_t x_ini, const uint32_t y_ini, const uint32_t w, const uint32_t h);
	Mylib::Matrix<TextureDescriptor> split_texture (const TextureDescriptor& texture__, const uint32_t n_rows, const uint32_t n_cols);

	// the following will generate an id randomly

	TextureDescriptor load_texture (const std::string_view fname)
	{
		return this->load_texture(this->find_unused_texture_id(), fname);
	}

	TextureDescriptor create_sub_texture (const TextureDescriptor& parent__, const uint32_t x_ini, const uint32_t y_ini, const uint32_t w, const uint32_t h)
	{
		return this->create_sub_texture(this->find_unused_texture_id(), parent__, x_ini, y_ini, w, h);
	}

	TextureDescriptor find_texture (const std::string_view id)
	{
		auto it = this->textures.find(id);
		mylib_assert_exception_args(it != this->textures.end(), TextureNotFoundException, id);
		return TextureDescriptor { .info = &it->second };
	}

	// light functions

	[[nodiscard]] LightPointDescriptor add_light_point_source (const Point& pos, const Color& color);
	
	inline void move_light_point_source (const LightPointDescriptor desc, const Point& pos)
	{
		LightPointSource& light_source = this->light_point_sources[desc];
		light_source.pos = pos;
	}

protected:
	virtual TextureInfo load_texture__ (SDL_Surface *surface) = 0;
	virtual void destroy_texture__ (TextureInfo& texture) = 0;
	virtual TextureInfo create_sub_texture__ (const TextureInfo& parent, const uint32_t x_ini, const uint32_t y_ini, const uint32_t w, const uint32_t h) = 0;

private:
	TextureInfo& add_texture (std::string id, const TextureInfo& texture__);
	std::string find_unused_texture_id ();
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