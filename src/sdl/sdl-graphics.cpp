#include <cstdlib>
#include <cmath>

#include <SDL.h>

#include <my-lib/math.h>
#include <my-lib/std.h>

#include <my-game-lib/debug.h>
#include <my-game-lib/sdl/sdl-driver.h>

// ---------------------------------------------------

//#define DEBUG_SHOW_CENTER_LINE

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

static SDL_Color to_sdl_color(const Color& color) noexcept
{
	auto calc = [](const float v) noexcept -> Uint8 {
		return static_cast<Uint8>(v * 255.0f);
	};
	return SDL_Color {
		.r = calc(color.r),
		.g = calc(color.g),
		.b = calc(color.b),
		.a = calc(color.a)
		};
}

// ---------------------------------------------------

#if 0
static void my_SDL_DrawCircle (SDL_Renderer *renderer, const int32_t centreX, const int32_t centreY, const int32_t radius)
{
	const int32_t diameter = (radius * 2);

	int32_t x = (radius - 1);
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y) {
		//  Each of the following renders an octant of the circle
		SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
		SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
		SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
		SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
		SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
		SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
		SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
		SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

		if (error <= 0) {
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0) {
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}
#endif

// ---------------------------------------------------

static void my_SDL_DrawFilledCircle (SDL_Renderer *renderer, const int32_t centreX, const int32_t centreY, const int32_t radius)
{
	for (int32_t w = 0; w < radius * 2; w++) {
		for (int32_t h = 0; h < radius * 2; h++) {
			int32_t dx = radius - w; // horizontal offset
			int32_t dy = radius - h; // vertical offset
			if ((dx*dx + dy*dy) <= (radius * radius))
				SDL_RenderDrawPoint(renderer, centreX + dx, centreY + dy);
		}
	}
}

// ---------------------------------------------------

SDL_GraphicsDriver::SDL_GraphicsDriver (const InitParams& params)
	: Manager (params)
{
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
			SDL_WINDOW_FULLSCREEN
		);

		dprintln("fullscrren window created with width=", this->window_width_px, " height=", this->window_height_px);
	}
	else
		this->sdl_window = SDL_CreateWindow(
			params.window_name.data(),
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			this->window_width_px, this->window_height_px,
			SDL_WINDOW_SHOWN);
	
	this->renderer = SDL_CreateRenderer(this->sdl_window, -1, SDL_RENDERER_ACCELERATED);

	dprintln("SDL renderer created");

	this->wait_next_frame();
}

// ---------------------------------------------------

SDL_GraphicsDriver::~SDL_GraphicsDriver ()
{
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->sdl_window);
}

// ---------------------------------------------------

void SDL_GraphicsDriver::wait_next_frame ()
{
	SDL_Color color = to_sdl_color(this->background_color);
	SDL_SetRenderDrawColor(this->renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(this->renderer);
}

// ---------------------------------------------------

void SDL_GraphicsDriver::draw_cube3D (Cube3D& cube, const Vector& offset, const Color& color)
{
	mylib_throw_exception_msg("SDL Renderer does not support 3D rendering");
}

// ---------------------------------------------------

void SDL_GraphicsDriver::draw_sphere3D (Sphere3D& sphere, const Vector& offset, const Color& color)
{
	mylib_throw_exception_msg("SDL Renderer does not support 3D rendering");
}

// ---------------------------------------------------

void SDL_GraphicsDriver::draw_circle2D (Circle2D& circle, const Vector& offset, const Color& color)
{
	const SDL_Color sdl_color = to_sdl_color(color);
	const Vector world_pos = offset;
	const Vector4 clip_pos = this->projection_matrix * Vector4(world_pos.x, world_pos.y, 0, 1);

/*	Game::ShapeRect rect(circle.get_radius()*2.0f, circle.get_radius()*2.0f);
	rect.set_delta(circle.get_delta());
	this->draw_rect(rect, offset, color);*/

	SDL_SetRenderDrawColor(this->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a);

	my_SDL_DrawFilledCircle(
		renderer,
		static_cast<int32_t>( clip_pos.x ),
		static_cast<int32_t>( clip_pos.y ),
		static_cast<int32_t>( circle.get_radius() * this->scale_factor )
		);
}

// ---------------------------------------------------

void SDL_GraphicsDriver::draw_rect2D (Rect2D& rect, const Vector& offset, const Color& color)
{
	SDL_Rect sdl_rect;
	const SDL_Color sdl_color = to_sdl_color(color);
	const Vector world_pos = offset;
	//const Vector world_pos = Vector(4.0f, 4.0f);
	const Vector4 clip_pos = this->projection_matrix * Vector4(world_pos.x, world_pos.y, 0, 1);
	//const Vector4d clip_pos = translate_to_clip_init * clip_pos_;

#if 0
	dprint( "world_pos:" )
	world_pos.println();

	dprint( "clip_pos:" )
	clip_pos.println();
//exit(1);
#endif

	sdl_rect.x = Mylib::Math::round_to_nearest<int>(clip_pos.x - (rect.get_w() * fp(0.5) * this->scale_factor));
	sdl_rect.y = Mylib::Math::round_to_nearest<int>(clip_pos.y - (rect.get_h() * fp(0.5) * this->scale_factor));
	sdl_rect.w = Mylib::Math::round_to_nearest<int>(rect.get_w() * this->scale_factor);
	sdl_rect.h = Mylib::Math::round_to_nearest<int>(rect.get_h() * this->scale_factor);

	SDL_SetRenderDrawColor(this->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a);
	SDL_RenderFillRect(this->renderer, &sdl_rect);
}

// ---------------------------------------------------

void SDL_GraphicsDriver::setup_render_3D (const RenderArgs3D& args)
{
	mylib_throw_exception_msg("SDL Renderer does not support 3D rendering");
}

// ---------------------------------------------------

void SDL_GraphicsDriver::setup_render_2D (const RenderArgs2D& args)
{
	using Vector = Vector2;
	using Point = Vector;

	const fp_t max_value = static_cast<fp_t>( std::max(this->window_width_px, this->window_height_px) );
	const Vector clip_init = args.clip_init_norm * max_value;
	const Vector clip_end = args.clip_end_norm * max_value;
	const Vector clip_size = clip_end - clip_init;
	const fp_t clip_aspect_ratio = clip_size.x / clip_size.y;

	const Vector world_size = args.world_end - args.world_init;
	
	const fp_t world_screen_width = std::min(args.world_screen_width, world_size.x);
	const fp_t world_screen_height = std::min(world_screen_width / clip_aspect_ratio, world_size.y);

	const Vector world_screen_size = Vector(world_screen_width, world_screen_height);

	this->scale_factor = clip_size.x / world_screen_size.x;

	Vector world_camera = args.world_camera_focus - Vector(world_screen_size.x*fp(0.5), world_screen_size.y*fp(0.5));

	//dprint( "world_camera PRE: " ) world_camera.println();

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
	dprintln( "clip_init: ", clip_init );
	dprintln( "clip_end: ", clip_end );
	dprintln( "clip_size: ", clip_size );
	dprintln( "clip_aspect_ratio: ", clip_aspect_ratio );
	dprintln( "scale_factor: ", this->scale_factor );
	dprintln( "world_size: ", world_size );
	dprintln( "world_screen_size: ", world_screen_size );
	dprintln( "args.world_camera_focus: ", args.world_camera_focus );
	dprintln( "world_camera: ", world_camera );
//exit(1);
#endif

	Matrix4 translate_to_clip_init;
	translate_to_clip_init.set_translate<2>(clip_init);
//	dprintln( "translation to clip init:" ); dprintln( translate_to_clip_init );

	Matrix4 scale;
	scale.set_scale<2>(Vector(this->scale_factor, this->scale_factor));
//	dprintln( "scale matrix:" ); dprintln( scale );
//exit(1);

	Matrix4 translate_camera;
	translate_camera.set_translate<2>(-world_camera);
//	dprintln( "translation matrix:" ); dprintln( translate_camera );

	this->projection_matrix = (translate_to_clip_init * scale) * translate_camera;
	//this->projection_matrix = scale * translate_camera;
//	dprintln( "final matrix:" ); dprintln( this->projection_matrix );
}

// ---------------------------------------------------

void SDL_GraphicsDriver::render ()
{
#ifdef DEBUG_SHOW_CENTER_LINE
{
	const SDL_Color sdl_color = { 255, 0, 0, 255 };

	SDL_SetRenderDrawColor(this->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a);
	SDL_RenderDrawLine(this->renderer, this->window_width_px / 2, 0, this->window_width_px / 2, this->window_height_px);
	SDL_RenderDrawLine(this->renderer, 0, this->window_height_px / 2, this->window_width_px, this->window_height_px / 2);
}
#endif
}

// ---------------------------------------------------

void SDL_GraphicsDriver::update_screen ()
{
	SDL_RenderPresent(this->renderer);
}

// ---------------------------------------------------

void SDL_GraphicsDriver::clear_vertex_buffers ()
{

}

// ---------------------------------------------------

} // namespace Graphics
} // namespace MyGlib