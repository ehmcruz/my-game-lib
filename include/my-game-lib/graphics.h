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

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/any.h>
#include <my-lib/memory.h>
#include <my-lib/math.h>
#include <my-lib/math-matrix.h>
#include <my-lib/math-geometry.h>

// ---------------------------------------------------

namespace MyGlib
{

// ---------------------------------------------------

using fp_t = MYGLIB_FP_TYPE;
using Vector2 = Mylib::Math::Vector<fp_t, 2>;
using Vector3 = Mylib::Math::Vector<fp_t, 3>;
using Vector4 = Mylib::Math::Vector<fp_t, 4>;
using Line = Mylib::Math::Line<fp_t, 3>;
using Matrix4 = Mylib::Math::Matrix<fp_t, 4, 4>;
using Point2 = Vector2;
using Point3 = Vector3;
using Point4 = Vector4;
using Vector = Vector3;
using Point = Vector;

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

	static consteval Color black ()
	{
		return Color{0.0f, 0.0f, 0.0f, 1.0f};
	}

	static consteval Color white ()
	{
		return Color{1.0f, 1.0f, 1.0f, 1.0f};
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
		Sphere3D,
	};
protected:
	OO_ENCAPSULATE_SCALAR_READONLY(Type, type)

	// distance from the center of the shape to the center of the object
	OO_ENCAPSULATE_OBJ_INIT(Vector, local_pos, Vector::zero())
	OO_ENCAPSULATE_SCALAR_INIT(fp_t, local_rotation_angle, 0)
	OO_ENCAPSULATE_OBJ_INIT(Vector, local_rotation_axis, Vector::zero())

	OO_ENCAPSULATE_SCALAR_INIT(fp_t, self_rotation_angle, 0)
	OO_ENCAPSULATE_OBJ_INIT(Vector, self_rotation_axis, Vector::zero())

public:
	Shape (const Type type_) noexcept
		: type(type_)
	{
	}

	constexpr void set_local_rotation_angle_bounded (const fp_t angle) noexcept
	{
		this->local_rotation_angle = std::fmod(angle, Mylib::Math::degrees_to_radians(fp(360)));
	}

	constexpr void set_self_rotation_angle_bounded (const fp_t angle) noexcept
	{
		this->self_rotation_angle = std::fmod(angle, Mylib::Math::degrees_to_radians(fp(360)));
	}
};

// ---------------------------------------------------

class Cube3D: public Shape
{
public:
	static consteval uint32_t get_n_vertices ()
	{
		return 8;
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
	OO_ENCAPSULATE_SCALAR(fp_t, w) // width
	std::array<Color, 8> colors;

public:
	Cube3D (const fp_t w_) noexcept
		: Shape(Type::Cube3D), w(w_)
	{
		//dprint( "circle created r=" << this->radius << std::endl )
	}

	Cube3D () noexcept
		: Cube3D(0)
	{
	}

	void set_vertex_color (const PositionIndex i, const Color& color) noexcept
	{
		this->colors[i] = color;
	}

	const Color& get_vertex_color (const PositionIndex i) const noexcept
	{
		return this->colors[i];
	}

	std::array<Color, 8>& get_colors_ref () noexcept
	{
		return this->colors;
	}

	inline fp_t get_h () const noexcept
	{
		return this->w;
	}

	inline fp_t get_d () const noexcept
	{
		return this->w;
	}
};

// ---------------------------------------------------

struct RenderArgs3D {
	Point world_camera_pos;
	Point world_camera_target;
	fp_t fov_y;
	fp_t z_near;
	fp_t z_far;
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

class GraphicsManager
{
public:
	enum class Type { // any change here will need a change in get_type_str
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
	
	OO_ENCAPSULATE_SCALAR_READONLY(fp_t, window_aspect_ratio)
	OO_ENCAPSULATE_OBJ_INIT(Color, background_color, Color::black())

	SDL_Window *sdl_window;

public:
	GraphicsManager (const InitParams& params)
		: memory_manager(params.memory_manager),
		window_width_px(params.window_width_px),
		window_height_px(params.window_height_px),
		fullscreen(params.fullscreen)
	{
		this->window_aspect_ratio = static_cast<fp_t>(this->window_width_px) / static_cast<fp_t>(this->window_height_px);
	}

	inline fp_t get_inverted_window_aspect_ratio () const
	{
		return fp(1) / this->window_aspect_ratio;
	}

	inline Vector2 get_normalized_window_size () const
	{
		const fp_t max_value = static_cast<fp_t>( std::max(this->window_width_px, this->window_height_px) );
		return Vector2(static_cast<fp_t>(this->window_width_px) / max_value, static_cast<fp_t>(this->window_height_px) / max_value);
	}

	virtual void wait_next_frame () = 0;
	virtual void draw_cube3D (const Cube3D& cube, const Vector& offset) = 0;
	virtual void setup_render_3D (const RenderArgs3D& args) = 0;
	virtual void setup_render_2D (const RenderArgs2D& args) = 0;
	virtual void render () = 0;
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif