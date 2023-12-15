#include <cstdlib>
#include <cmath>

#include <my-lib/math.h>
#include <my-lib/std.h>

#include <my-game-lib/debug.h>
#include <my-game-lib/sdl/sdl-driver.h>

// ---------------------------------------------------

#define DEBUG_SHOW_CENTER_LINE

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

// ---------------------------------------------------


// ---------------------------------------------------

// ---------------------------------------------------

SDL_Renderer::SDL_Renderer (const InitParams& params)
	: Manager (params)
{

}

// ---------------------------------------------------

SDL_Renderer::~SDL_Renderer ()
{
}

// ---------------------------------------------------

void SDL_Renderer::wait_next_frame ()
{

}

// ---------------------------------------------------

void SDL_Renderer::draw_cube3D (const Cube3D& cube, const Vector& offset, const Color& color)
{
	mylib_throw_exception_msg("SDL Renderer does not support 3D rendering");
}

// ---------------------------------------------------

void Renderer::draw_circle2D (const Circle2D& circle, const Vector& offset, const Color& color)
{

}

// ---------------------------------------------------

void Renderer::draw_rect2D (const Rect2D& rect, const Vector& offset, const Color& color)
{

}

// ---------------------------------------------------

void Renderer::setup_render_3D (const RenderArgs3D& args)
{
	mylib_throw_exception_msg("SDL Renderer does not support 3D rendering");
}

// ---------------------------------------------------

void Renderer::setup_render_2D (const RenderArgs2D& args)
{

}

// ---------------------------------------------------

void Renderer::render ()
{

}

// ---------------------------------------------------

void SDL_Renderer::update_screen ()
{

}

// ---------------------------------------------------

void SDL_Renderer::clear_vertex_buffers ()
{

}

// ---------------------------------------------------

} // namespace Graphics
} // namespace MyGlib