#ifndef __MY_GAME_LIB_SDL_HEADER_H__
#define __MY_GAME_LIB_SDL_HEADER_H__

#include <my-game-lib/my-game-lib.h>
#include <my-game-lib/audio.h>

#include <my-lib/memory.h>

// ---------------------------------------------------

namespace MyGlib
{

// ---------------------------------------------------

void SDL_Driver_Init ();
void SDL_Driver_End ();

// ---------------------------------------------------

namespace Event
{
	class SDL_EventDriver : public Manager
	{
	public:
		SDL_EventDriver (Mylib::Memory::Manager& memory_manager_);

		void process_events () override final;
	};
} // end namespace Event

// ---------------------------------------------------

namespace Audio
{
	class SDL_AudioDriver : public Manager
	{
	public:
		SDL_AudioDriver (Mylib::Memory::Manager& memory_manager_);
		~SDL_AudioDriver ();
		
	public:
		Descriptor load_sound (const std::string_view fname, const Format format) override final;
		Descriptor load_music (const std::string_view fname, const Format format) override final;
		void unload_audio (Descriptor& audio) override final;
		void driver_play_audio (Descriptor& audio, Callback *callback) override final;
		void set_volume (Descriptor& audio, const float volume) override final;
	};
} // end namespace Audio

// ---------------------------------------------------

namespace Graphics
{
	class SDL_GraphicsDriver : public Manager
	{
	private:
		SDL_Renderer *renderer;
		SDL_RendererInfo renderer_info;
		Matrix4 projection_matrix;
		fp_t scale_factor;

	public:
		SDL_GraphicsDriver (const InitParams& params);
		~SDL_GraphicsDriver ();

		void wait_next_frame () override final;
		void draw_cube3D (Cube3D& cube, const Vector& offset, const Color& color) override final;
		void draw_cube3D (Cube3D& cube, const Vector& offset, const std::array<TextureRenderOptions, 6>& texture_options) override final;
		void draw_sphere3D (Sphere3D& sphere, const Vector& offset, const Color& color) override final;
		void draw_sphere3D (Sphere3D& sphere, const Vector& offset, const TextureRenderOptions& texture_options) override final;
		void draw_circle2D (Circle2D& circle, const Vector& offset, const Color& color) override final;
		void draw_rect2D (Rect2D& rect, const Vector& offset, const Color& color) override final;
		void draw_rect2D (Rect2D& rect, const Vector& offset, const TextureRenderOptions& texture_options) override final;
		void setup_render_3D (const RenderArgs3D& args) override final;
		void setup_render_2D (const RenderArgs2D& args) override final;
		void render () override final;
		void update_screen () override final;
		void clear_buffers (const uint32_t flags) override final;

		void begin_texture_loading () override final;
		void end_texture_loading () override final;
		TextureDescriptor load_texture (SDL_Surface *surface) override final;
		void destroy_texture (TextureDescriptor& texture) override final;
	
	private:
		SDL_Rect helper_calc_sdl_rect (Rect2D& rect, const Vector& world_pos);
	};
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif