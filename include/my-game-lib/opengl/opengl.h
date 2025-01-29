#ifndef __MY_GAME_LIB_GRAPHICS_OPENGL_HEADER_H__
#define __MY_GAME_LIB_GRAPHICS_OPENGL_HEADER_H__

#ifdef __MINGW32__
	#define SDL_MAIN_HANDLED
#endif

#ifndef __ANDROID__
	#include <GL/glew.h>
#endif

#include <SDL.h>

#ifndef __ANDROID__
	#include <SDL_opengl.h>
#else
	#include <SDL_opengles2.h>
	#include <GLES3/gl3.h>
#endif

#include <cstring>

#include <string>
#include <string_view>
#include <span>
#include <vector>
#include <list>

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/matrix.h>

#include <my-game-lib/graphics.h>
#include <my-game-lib/texture-atlas.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{
namespace Opengl
{

// ---------------------------------------------------

struct Opengl_AtlasDescriptor
{
	// This is actually an integer.
	// It's used to store the texture id of the atlas.
	// But we use a float because it is passed as a z-coordinate to the shaders.
	float texture_depth;
	int32_t width_px;
	int32_t height_px;
};

struct Opengl_TextureDescriptor
{
	SDL_Surface *surface;
	Opengl_AtlasDescriptor *atlas;
	int32_t width_px;
	int32_t height_px;
	Vector2f tex_coords[4];
};

// ---------------------------------------------------

void ensure_no_error ();

// ---------------------------------------------------

class Program;

class Shader
{
protected:
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLuint, shader_id)
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLenum, shader_type)
	MYLIB_OO_ENCAPSULATE_OBJ_READONLY(std::string, fname)

public:
	Shader (const GLenum shader_type_, const std::string_view fname_);
	~Shader ();
	void compile ();
};

// ---------------------------------------------------

class Program
{
protected:
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLuint, program_id)
	MYLIB_OO_ENCAPSULATE_PTR_INIT(Shader*, vs, nullptr)
	MYLIB_OO_ENCAPSULATE_PTR_INIT(Shader*, fs, nullptr)

protected:
	Program ();
	~Program ();
	void attach_shaders ();
	void link_program ();
	void use_program ();
	GLint get_uniform_location (const std::string_view name) const;
	void bind_attrib_location (const GLuint index, const std::string_view name);
	void gen_vertex_arrays (const GLsizei n, GLuint *arrays);
	void gen_buffers (const GLsizei n, GLuint *buffers);
	void bind_vertex_array (const GLuint array);
	void bind_buffer (const GLenum target, const GLuint buffer);
	void enable_vertex_attrib_array (const GLuint index);
};

// ---------------------------------------------------

template <typename T>
class VertexBuffer
{
protected:
	MYLIB_OO_ENCAPSULATE_SCALAR_INIT(uint32_t, grow_factor, 8*1024)
	MYLIB_OO_ENCAPSULATE_PTR_INIT(T*, vertex_buffer, nullptr)
	MYLIB_OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint32_t, vertex_buffer_used, 0)
	MYLIB_OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint32_t, vertex_buffer_capacity, 0)

	void realloc (const uint32_t target_capacity)
	{
		uint32_t old_capacity = this->vertex_buffer_capacity;
		T *old_buffer = this->vertex_buffer;

		this->vertex_buffer_capacity += this->grow_factor;

		if (this->vertex_buffer_capacity < target_capacity)
			this->vertex_buffer_capacity = target_capacity;
		this->vertex_buffer = new T[this->vertex_buffer_capacity];

		memcpy(this->vertex_buffer, old_buffer, old_capacity * sizeof(T));

		delete[] old_buffer;
	}

public:
	VertexBuffer ()
	{
		this->vertex_buffer_capacity = this->grow_factor; // can't be zero
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

class ProgramTriangleColor : public Program
{
protected:
	enum AttribIndex {
		iPosition,
		iNormal,
		iOffset,
		iColor
	};

	GLint u_projection_matrix;
	GLint u_ambient_light_color;
	GLint u_point_light_pos;
	GLint u_point_light_color;

public:
	struct Uniforms {
		Matrix4 projection_matrix;
		Color ambient_light_color;
		Point point_light_pos;
		Color point_light_color;
	};

	struct Vertex {
		Graphics::Vertex gvertex;
		Vector offset; // global x,y,z coords, which are added to the local coords
		Color color; // rgba
	};

	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLuint, vao) // vertex array descriptor id
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLuint, vbo) // vertex buffer id

protected:
	VertexBuffer<Vertex> triangle_buffer;

public:
	ProgramTriangleColor ();
	~ProgramTriangleColor ();

	inline void clear ()
	{
		this->triangle_buffer.clear();
	}

	inline std::span<Vertex> alloc_vertices (const uint32_t n)
	{
		return this->triangle_buffer.alloc_vertices(n);
	}

	inline bool has_vertices () const noexcept
	{
		return (this->triangle_buffer.get_vertex_buffer_used() > 0);
	}

	void bind_vertex_arrays ();
	void bind_vertex_buffers ();
	void setup_vertex_arrays ();
	void setup_uniforms ();
	void upload_vertex_buffers ();
	void upload_uniforms (const Uniforms& uniforms);
	void draw ();
	void load ();
	void debug ();
};

// ---------------------------------------------------

class ProgramTriangleTexture : public Program
{
protected:
	enum AttribIndex {
		iPosition,
		iNormal,
		iOffset,
		iTexCoords
	};

	GLint u_projection_matrix;
	GLint u_ambient_light_color;
	GLint u_point_light_pos;
	GLint u_point_light_color;
	GLint u_tx_unit;

public:
	struct Uniforms {
		Matrix4 projection_matrix;
		Color ambient_light_color;
		Point point_light_pos;
		Color point_light_color;
	};

	struct Vertex {
		Graphics::Vertex gvertex;
		Vector offset; // global x,y,z coords, which are added to the local coords
		Point3f tex_coords;
	};

	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLuint, vao) // vertex array descriptor id
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLuint, vbo) // vertex buffer id

protected:
	VertexBuffer<Vertex> triangle_buffer;

public:
	ProgramTriangleTexture ();
	~ProgramTriangleTexture ();

	inline void clear ()
	{
		this->triangle_buffer.clear();
	}

	inline std::span<Vertex> alloc_vertices (const uint32_t n)
	{
		return this->triangle_buffer.alloc_vertices(n);
	}

	inline bool has_vertices () const noexcept
	{
		return (this->triangle_buffer.get_vertex_buffer_used() > 0);
	}

	void bind_vertex_arrays ();
	void bind_vertex_buffers ();
	void setup_vertex_arrays ();
	void setup_uniforms ();
	void upload_vertex_buffers ();
	void upload_uniforms (const Uniforms& uniforms);
	void draw ();
	void load ();
	void debug ();
};

// ---------------------------------------------------

class ProgramTriangleTextureRotation : public Program
{
protected:
	enum AttribIndex {
		iPosition,
		iNormal,
		iOffset,
		iTexCoords,
		iRotQuat
	};

	GLint u_projection_matrix;
	GLint u_ambient_light_color;
	GLint u_point_light_pos;
	GLint u_point_light_color;
	GLint u_tx_unit;

public:
	using Uniforms = ProgramTriangleTexture::Uniforms;

	struct Vertex {
		Graphics::Vertex gvertex;
		Vector offset; // global x,y,z coords, which are added to the local coords
		Point3f tex_coords;
		Quaternion rot_quat;
	};

	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLuint, vao) // vertex array descriptor id
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(GLuint, vbo) // vertex buffer id

protected:
	VertexBuffer<Vertex> triangle_buffer;

public:
	ProgramTriangleTextureRotation ();
	~ProgramTriangleTextureRotation ();

	inline void clear ()
	{
		this->triangle_buffer.clear();
	}

	inline std::span<Vertex> alloc_vertices (const uint32_t n)
	{
		return this->triangle_buffer.alloc_vertices(n);
	}

	inline bool has_vertices () const noexcept
	{
		return (this->triangle_buffer.get_vertex_buffer_used() > 0);
	}

	void bind_vertex_arrays ();
	void bind_vertex_buffers ();
	void setup_vertex_arrays ();
	void setup_uniforms ();
	void upload_vertex_buffers ();
	void upload_uniforms (const Uniforms& uniforms);
	void draw ();
	void load ();
	void debug ();
};

// ---------------------------------------------------

class Renderer : public Manager
{
protected:
	static inline constexpr int32_t max_texture_size = 4096;

protected:
	SDL_GLContext sdl_gl_context;

	ProgramTriangleColor::Uniforms program_triangle_color_uniforms;
	ProgramTriangleColor *program_triangle_color;

	ProgramTriangleTexture::Uniforms program_triangle_texture_uniforms;
	ProgramTriangleTexture *program_triangle_texture;
	ProgramTriangleTextureRotation *program_triangle_texture_rotation;
	
	std::vector<TextureDescriptor> textures;
	std::list<Opengl_AtlasDescriptor> atlases;
	GLuint texture_array_id;

public:
	Renderer (const InitParams& params);
	~Renderer ();

	void wait_next_frame () override final;
	void draw_cube3D (Cube3D& cube, const Vector& offset, const Color& color) override final;
	void draw_cube3D (Cube3D& cube, const Vector& offset, const std::array<TextureRenderOptions, 6>& texture_options) override final;
	void draw_sphere3D (Sphere3D& sphere, const Vector& offset, const Color& color) override final;
	void draw_sphere3D (Sphere3D& sphere, const Vector& offset, const TextureRenderOptions& texture_options) override final;
	void draw_circle2D (Circle2D& circle, const Vector& offset, const Color& color) override final;
	void draw_rect2D (Rect2D& rect, const Vector& offset, const Color& color) override final;
	void draw_rect2D (Rect2D& rect, const Vector& offset, const TextureRenderOptions& texture_optionss) override final;
	void setup_render_3D (const RenderArgs3D& args) override final;
	void setup_render_2D (const RenderArgs2D& args) override final;
	void render () override final;
	void update_screen () override final;
	void clear_buffers (const uint32_t flags) override final;
	
	void begin_texture_loading () override final;
	void end_texture_loading () override final;
	TextureDescriptor load_texture (SDL_Surface *surface) override final;
	void destroy_texture (TextureDescriptor& texture) override final;

	void load_opengl_programs ();
};

// ---------------------------------------------------

} // end namespace Opengl
} // end namespace Graphics
} // end namespace MyGlib

#endif