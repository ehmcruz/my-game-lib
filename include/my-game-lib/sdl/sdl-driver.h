#ifndef __MY_GAME_LIB_SDL_HEADER_H__
#define __MY_GAME_LIB_SDL_HEADER_H__

#include <my-game-lib/my-game-lib.h>
#include <my-game-lib/audio.h>

#include <my-lib/memory.h>


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
		Matrix4 projection_matrix;
		fp_t scale_factor;

	public:
		SDL_GraphicsDriver (const InitParams& params);
		~SDL_GraphicsDriver ();

		void wait_next_frame () override final;
		void draw_cube3D (const Cube3D& cube, const Vector& offset, const Color& color) override final;
		void draw_sphere3D (const Sphere3D& sphere, const Vector& offset, const Color& color) override final;
		void draw_circle2D (const Circle2D& circle, const Vector& offset, const Color& color) override final;
		void draw_rect2D (const Rect2D& rect, const Vector& offset, const Color& color) override final;
		void setup_render_3D (const RenderArgs3D& args) override final;
		void setup_render_2D (const RenderArgs2D& args) override final;
		void render () override final;
		void update_screen () override final;
		void clear_vertex_buffers () override final;
	};
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif