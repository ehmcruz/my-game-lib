#ifndef __MY_GAME_LIB_GRAPHICS_OPENGL_HEADER_H__
#define __MY_GAME_LIB_GRAPHICS_OPENGL_HEADER_H__

#ifdef __MINGW32__
	#define SDL_MAIN_HANDLED
#endif

#include <GL/glew.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <cstring>

#include <string>
#include <string_view>
#include <span>

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/matrix.h>

#include <my-game-lib/graphics.h>


namespace MyGlib
{
namespace Graphics
{
namespace Opengl
{

// ---------------------------------------------------

//#define MYGLIB_OPENGL_SOFTWARE_CALCULATE_MATRIX

// ---------------------------------------------------

class Program;

// ---------------------------------------------------

class Shader
{
protected:
	OO_ENCAPSULATE_SCALAR_READONLY(GLuint, shader_id)
	OO_ENCAPSULATE_SCALAR_READONLY(GLenum, shader_type)
	OO_ENCAPSULATE_OBJ_READONLY(std::string, fname)

public:
	Shader (const GLenum shader_type_, const std::string_view fname_);
	void compile ();

	friend class Program;
};

// ---------------------------------------------------

class Program
{
protected:
	OO_ENCAPSULATE_SCALAR_READONLY(GLuint, program_id)
	OO_ENCAPSULATE_PTR(Shader*, vs)
	OO_ENCAPSULATE_PTR(Shader*, fs)

public:
	Program ();
	void attach_shaders ();
	void link_program ();
	void use_program ();
};

// ---------------------------------------------------

template <typename T, uint32_t grow_factor=4096>
class VertexBuffer
{
protected:
	OO_ENCAPSULATE_PTR_INIT(T*, vertex_buffer, nullptr)
	OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint32_t, vertex_buffer_used, 0)
	OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint32_t, vertex_buffer_capacity, 0)

	void realloc (const uint32_t target_capacity)
	{
		uint32_t old_capacity = this->vertex_buffer_capacity;
		T *old_buffer = this->vertex_buffer;

		this->vertex_buffer_capacity += grow_factor;

		if (this->vertex_buffer_capacity < target_capacity)
			this->vertex_buffer_capacity = target_capacity;
		this->vertex_buffer = new T[this->vertex_buffer_capacity];

		memcpy(this->vertex_buffer, old_buffer, old_capacity * sizeof(T));

		delete[] old_buffer;
	}

public:
	VertexBuffer ()
	{
		static_assert(grow_factor > 0);

		this->vertex_buffer_capacity = grow_factor; // can't be zero
		this->vertex_buffer = new T[this->vertex_buffer_capacity];

		this->vertex_buffer_used = 0;
	}

	~VertexBuffer ()
	{
		if (this->vertex_buffer != nullptr) {
			delete[] this->vertex_buffer;
			this->vertex_buffer = nullptr;
		}
	}

	inline T& get_vertex (const uint32_t i) noexcept
	{
		return *(this->vertex_buffer + i);
	}

	inline std::span<T> alloc_vertices (const uint32_t n)
	{
		const uint32_t free_space = this->vertex_buffer_capacity - this->vertex_buffer_used;

		if (free_space < n) [[unlikely]]
			this->realloc(this->vertex_buffer_used + n);
		
		T *vertices = this->vertex_buffer + this->vertex_buffer_used;
		this->vertex_buffer_used += n;

		return std::span<T>{vertices, n};
	}

	inline void clear () noexcept
	{
		this->vertex_buffer_used = 0;
	}
};

// ---------------------------------------------------

class ProgramTriangle: public Program
{
protected:
	enum class Attrib : uint32_t {
		Position,
		Normal,
		Offset,
		Color
	};

public:
	struct Vertex {
	#ifndef MYGLIB_OPENGL_SOFTWARE_CALCULATE_MATRIX
		Graphics::Vertex gvertex;
	#else
		Point4 local_pos; // local x,y,z coords
	#endif
		Vector offset; // global x,y,z coords, which are added to the local coords
		Color color; // rgba
	};

	OO_ENCAPSULATE_SCALAR_READONLY(GLuint, vao) // vertex array descriptor id
	OO_ENCAPSULATE_SCALAR_READONLY(GLuint, vbo) // vertex buffer id

protected:
	VertexBuffer<Vertex, 8192> triangle_buffer;

public:
	ProgramTriangle ();

	/*consteval static uint32_t get_stride_in_floats ()
	{
		return (sizeof(Vertex) / sizeof(GLfloat));
	}*/

	inline void clear ()
	{
		this->triangle_buffer.clear();
	}

	inline std::span<ProgramTriangle::Vertex> alloc_vertices (const uint32_t n)
	{
		return this->triangle_buffer.alloc_vertices(n);
	}

	void bind_vertex_array ();
	void bind_vertex_buffer ();
	void setup_vertex_array ();
	void upload_vertex_buffer ();

	void upload_uniforms (const Matrix4& projection_matrix,
	                      const Color& ambient_light
	                     );

	void draw ();

	void debug ();
};

// ---------------------------------------------------

class Renderer : public Manager
{
protected:
	SDL_GLContext sdl_gl_context;
	Matrix4 projection_matrix;
	Color ambient_light;

	ProgramTriangle *program_triangle;

public:
	Renderer (const InitParams& params);
	~Renderer ();

	void wait_next_frame () override final;
	void draw_cube3D (const Cube3D& cube, const Vector& offset, const Color& color) override final;
	void draw_circle2D (const Circle2D& circle, const Vector& offset, const Color& color) override final;
	void draw_rect2D (const Rect2D& rect, const Vector& offset, const Color& color) override final;
	void setup_render_3D (const RenderArgs3D& args) override final;
	void setup_render_2D (const RenderArgs2D& args) override final;
	void render () override final;
	void update_screen () override final;
	void clear_vertex_buffers () override final;

	void load_opengl_programs ();
};

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace Opengl
} // end namespace MyGlib

#endif