#include <thread>

#include <my-game-lib/game/game.h>

// ---------------------------------------------------



// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

Main::Main (const InitConfig& config_, Entity *entity_)
	: config(config_), entity(entity_)
{
	this->state = State::Initializing;

	game_lib = &MyGlib::Lib::init({
		.graphics_type = MyGlib::Graphics::Manager::Type::Opengl,
		.window_name = this->config.window_name,
		.window_width_px = this->config.window_width_px,
		.window_height_px = this->config.window_height_px,
		.fullscreen = this->config.fullscreen
	});

	event_manager = &game_lib->get_event_manager();
	audio_manager = &game_lib->get_audio_manager();
	renderer = &game_lib->get_graphics_manager();

	std::random_device rd;
	random_generator.seed( rd() );

	//dprintln("chorono resolution ", (static_cast<float>(Clock::period::num) / static_cast<float>(Clock::period::den)));

	this->alive = true;

	this->event_quit_d = event_manager->quit().subscribe( Mylib::Event::make_callback_object<MyGlib::Event::Quit::Type>(*this, &Main::event_quit) );
	this->event_key_down_d = event_manager->key_down().subscribe( Mylib::Event::make_callback_object<MyGlib::Event::KeyDown::Type>(*this, &Main::event_key_down_callback) );
}

// ---------------------------------------------------

Main::~Main ()
{
	event_manager->quit().unsubscribe(this->event_quit_d);
	event_manager->key_down().unsubscribe(this->event_key_down_d);
	MyGlib::Lib::quit();
}

// ---------------------------------------------------

Game::Main* Main::load (const InitConfig& config_, Entity *entity_)
{
	instance = new Main(config_, entity_);
	return instance;
}

// ---------------------------------------------------

void Main::unload ()
{
	delete instance;
	instance = nullptr;
}

// ---------------------------------------------------

void Main::event_quit (const MyGlib::Event::Quit::Type)
{
	this->alive = false;
}

// ---------------------------------------------------

void Main::event_key_down_callback (const MyGlib::Event::KeyDown::Type& event)
{
	switch (event.key_code)
	{
		case SDLK_ESCAPE:
			this->alive = false;
		break;
	
		default:
			break;
	}
}

// ---------------------------------------------------

void Main::run ()
{
	float real_dt, virtual_dt, required_dt, sleep_dt, busy_wait_dt, fps;

	this->state = State::Playing;

	real_dt = 0.0f;
	virtual_dt = 0.0f;
	required_dt = 0.0f;
	sleep_dt = 0.0f;
	busy_wait_dt = 0.0f;
	fps = 0.0f;

	while (this->alive) {
		const ClockTime tbegin = Clock::now();
		ClockTime tend;
		ClockDuration elapsed;

		renderer->wait_next_frame();

		virtual_dt = (real_dt > this->config.max_dt) ? this->config.max_dt : real_dt;

		timer.trigger_events();
		interpolation_manager.process_interpolation(virtual_dt);

	#if 0
		dprintln("start new frame render target_dt=", Config::target_dt,
			" required_dt=", required_dt,
			" real_dt=", real_dt,
			" sleep_dt=", sleep_dt,
			" busy_wait_dt=", busy_wait_dt,
			" virtual_dt=", virtual_dt,
			" max_dt=", Config::max_dt,
			" target_dt=", Config::target_dt,
			" fps=", fps
			);
	#endif

		event_manager->process_events();

		switch (this->state) {
			case State::Playing:
				this->entity->loop_update(virtual_dt);
				this->entity->loop_physics(virtual_dt);
				this->entity->loop_render(virtual_dt);
			break;
			
			default:
				mylib_assert(0)
		}

		renderer->render();
		renderer->update_screen();

		switch (this->state) {
			case State::Playing:
				this->entity->frame_finished();
			break;
			
			default:
				mylib_assert(0)
		}

		const ClockTime trequired = Clock::now();
		elapsed = trequired - tbegin;
		required_dt = ClockDuration_to_float(elapsed);

		if (this->config.sleep_to_save_cpu) {
			if (required_dt < this->config.sleep_threshold) {
				sleep_dt = this->config.sleep_threshold - required_dt; // target sleep time
				std::this_thread::sleep_for(float_to_ClockDuration(sleep_dt));
			}
		}
		
		const ClockTime tbefore_busy_wait = Clock::now();
		elapsed = tbefore_busy_wait - trequired;
		sleep_dt = ClockDuration_to_float(elapsed); // check exactly time sleeping

		do {
			tend = Clock::now();
			elapsed = tend - tbegin;
			real_dt = ClockDuration_to_float(elapsed);

			if (!this->config.busy_wait_to_ensure_fps)
				break;
		} while (real_dt < this->config.target_dt);

		elapsed = tend - tbefore_busy_wait;
		busy_wait_dt = ClockDuration_to_float(elapsed);

		fps = 1.0f / real_dt;
	}
}

// ---------------------------------------------------

} // namespace Game
} // namespace MyGlib