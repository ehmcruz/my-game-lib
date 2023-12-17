#include <my-game-lib/my-game-lib.h>
#include <my-game-lib/debug.h>
#include <my-game-lib/sdl/sdl-driver.h>

#include <my-lib/macros.h>
#include <my-lib/trigger.h>

#include <SDL.h>


namespace MyGlib
{

// ---------------------------------------------------

void SDL_Driver_Init ()
{
#ifdef __ANDROID__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	dprintln("Initializing SDL Video and Audio ...");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		mylib_throw_exception_msg("SDL Video and Audio could not initialize! SDL_Error: ", SDL_GetError());

	dprintln("SDL Video and Audio Initialized");
}

// ---------------------------------------------------

void SDL_Driver_End ()
{
	SDL_Quit();
}

// ---------------------------------------------------

namespace Event
{

// ---------------------------------------------------

struct FingerEvent {
	bool free = true;
	SDL_TouchID touch_id;
	SDL_FingerID finger_id;
	uint64_t global_id;
	Graphics::Vector2f norm_down_pos;
	Graphics::Vector2f norm_last_pos;
};

static std::vector<FingerEvent> finger_events;
static uint64_t global_touch_id = 0;

// ---------------------------------------------------

SDL_EventDriver::SDL_EventDriver (Mylib::Memory::Manager& memory_manager_)
	: Manager(memory_manager_)
{
}

// ---------------------------------------------------

static void finger_event_check_trigger (SDL_EventDriver& sdl_driver, const FingerEvent& fe)
{
	const Graphics::Manager& graphics = Lib::get_instance().get_graphics_manager();

	const float dx = fe.norm_last_pos.x - fe.norm_down_pos.x;
	const float dy = fe.norm_last_pos.y - fe.norm_down_pos.y;
	constexpr float norm_threshold = 0.2f;

	const uint32_t window_width_px = graphics.get_window_width_px();
	const uint32_t window_height_px = graphics.get_window_height_px();

	const float dx_px = dx * static_cast<float>(window_width_px);
	const float dy_px = dy * static_cast<float>(window_height_px);

	const float threshold = norm_threshold * static_cast<float>(std::min(window_width_px, window_height_px));

	if (std::abs(dx_px) > threshold) {
		if (dx_px < 0.0f)
			sdl_driver.touch_screen_move().publish(TouchScreenMove::Type { .direction = TouchScreenMove::Type::Direction::Left });
		else
			sdl_driver.touch_screen_move().publish(TouchScreenMove::Type { .direction = TouchScreenMove::Type::Direction::Right });
	}
	else if (std::abs(dy_px) > threshold) {
		if (dy_px < 0.0f)
			sdl_driver.touch_screen_move().publish(TouchScreenMove::Type { .direction = TouchScreenMove::Type::Direction::Up });
		else
			sdl_driver.touch_screen_move().publish(TouchScreenMove::Type { .direction = TouchScreenMove::Type::Direction::Down });
	}
}

// ---------------------------------------------------

static FingerEvent& find_finger_event (const SDL_TouchID touch_id, const SDL_FingerID finger_id)
{
	for (auto& event : finger_events) {
		if (event.touch_id == touch_id && event.finger_id == finger_id)
			return event;
	}

	mylib_throw_exception_msg("finger event not found: touch_id=", touch_id, " finger_id=", finger_id);
}

// ---------------------------------------------------

static void process_fingerdown (SDL_EventDriver& sdl_driver, const SDL_TouchFingerEvent& event)
{
	FingerEvent *fe = nullptr;

	// check for a free slot
	for (auto& event : finger_events) {
		if (event.free) {
			fe = &event;
			break;
		}
	}

	if (fe == nullptr) // no free slot, allocate one
		fe = &finger_events.emplace_back();
	
	fe->free = false;
	fe->touch_id = event.touchId;
	fe->finger_id = event.fingerId;
	fe->global_id = global_touch_id++;
	fe->norm_down_pos = Graphics::Vector2f(event.x, event.y);
	fe->norm_last_pos = fe->norm_down_pos;
}

// ---------------------------------------------------

static void process_fingermotion (SDL_EventDriver& sdl_driver, const SDL_TouchFingerEvent& event_)
{

}

// ---------------------------------------------------

static void process_fingerup (SDL_EventDriver& sdl_driver, const SDL_TouchFingerEvent& event_)
{
	FingerEvent& fe = find_finger_event(event_.touchId, event_.fingerId);

	fe.norm_last_pos = Graphics::Vector2f(event_.x, event_.y);
	finger_event_check_trigger(sdl_driver, fe);

	fe.free = true;
}

// ---------------------------------------------------

void SDL_EventDriver::process_events ()
{
	SDL_Event sdl_event;

	while (SDL_PollEvent(&sdl_event)) {
		this->event_sdl.publish(sdl_event);

		switch (sdl_event.type) {
			case SDL_QUIT:
				this->event_quit.publish( { } );
			break;

			case SDL_KEYDOWN:
				this->event_key_down.publish( KeyDown::Type {
					.key_code = sdl_event.key.keysym.sym,
					.scan_code = sdl_event.key.keysym.scancode,
					.modifiers = sdl_event.key.keysym.mod
				} );
			break;

			case SDL_FINGERMOTION:
				process_fingermotion(*this, sdl_event.tfinger);
			break;

			case SDL_FINGERDOWN:
				process_fingerdown(*this, sdl_event.tfinger);
			break;

			case SDL_FINGERUP:
				process_fingerup(*this, sdl_event.tfinger);
			break;
		}
	}
}

// ---------------------------------------------------

} // end namespace Event

// ---------------------------------------------------

} // end namespace MyGlib