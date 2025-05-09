#ifndef __MY_GAME_LIB_GAME_HEADER_H__
#define __MY_GAME_LIB_GAME_HEADER_H__

#ifdef __MINGW32__
	#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>

#include <memory>
#include <list>
#include <utility>
#include <type_traits>
#include <chrono>

#include <cmath>
#include <cstdlib>

#include <my-game-lib/my-game-lib.h>
#include <my-game-lib/graphics.h>
#include <my-game-lib/events.h>

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/event.h>
#include <my-lib/event-timer.h>
#include <my-lib/interpolation.h>

// ---------------------------------------------------

// Common objects used in game engines.
// Basically, we have a component system where each entity can have multiple components.
// Each component can be of different types, and each type can have different subtypes.
// Not entirely based on the ECS (Entity Component System) model, since that are a few
// key Entitiy types to differentiate their physics processing.

// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

// Use Mylib's unique_ptr because it is compatible custom memory manager
// for polymorphic types.
template <typename T>
using unique_ptr = typename Mylib::Memory::unique_ptr<T>;

using Clock = std::chrono::steady_clock;
using ClockDuration = Clock::duration;
using ClockTime = Clock::time_point;

using Coroutine = Mylib::Coroutine<1024>;

using Graphics::Color;
using Graphics::TextureDescriptor;

// ---------------------------------------------------

inline MyGlib::Lib *game_lib = nullptr;
inline MyGlib::Event::Manager *event_manager = nullptr;
inline MyGlib::Audio::Manager *audio_manager = nullptr;
inline MyGlib::Graphics::Manager *renderer = nullptr;

inline std::mt19937_64 random_generator;

inline auto timer = Mylib::Event::make_timer<Coroutine>(Clock::now);
using Timer = decltype(timer);

inline Mylib::InterpolationManager<Coroutine, float> interpolation_manager;

// ---------------------------------------------------

class Entity;

class Component
{
protected:
	MYLIB_OO_ENCAPSULATE_PTR_READONLY(Entity*, parent)

public:
	Component (Entity *parent_)
		: parent(parent_)
	{
	}

	virtual ~Component () { };

	// called for each rendered frame
	virtual void process_render (const float dt) { };

	// called for each physics frame, before process_physics
	virtual void process_update (const float dt) { };

	// called for each physics frame
	virtual void process_physics (const float dt) { };

	// called after all the components have been processed and rendered
	virtual void frame_finished () { };
};

// ---------------------------------------------------

class Entity : public Component
{
public:
	using UserData = uint64_t;

public:
	UserData user_data;

protected:
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<unique_ptr<Component>>, components)
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<unique_ptr<Entity>>, entities)

public:
	Entity (Entity *parent_, const UserData& user_data_)
		: Component(parent_),
		  user_data(user_data_)
	{
	}

	virtual ~Entity () { };

	template <typename T>
	void add_child (unique_ptr<T> child)
	{
		if constexpr (std::is_base_of_v<Entity, T>)
			this->entities.push_back(std::move(child));
		else if constexpr (std::is_base_of_v<Component, T>)
			this->components.push_back(std::move(child));
		else
			static_assert(0);
	}

	void loop_render (const float dt)
	{
		for (auto& component : this->components) {
			component->process_render(dt);
		}

		for (auto& entity : this->entities) {
			entity->loop_render(dt);
		}

		this->process_render(dt);
	}

	void loop_update (const float dt)
	{
		for (auto& component : this->components) {
			component->process_update(dt);
		}

		for (auto& entity : this->entities) {
			entity->loop_update(dt);
		}

		this->process_update(dt);
	}

	void loop_physics (const float dt)
	{
		for (auto& component : this->components) {
			component->process_physics(dt);
		}

		for (auto& entity : this->entities) {
			entity->loop_physics(dt);
		}

		this->process_physics(dt);
	}
};

// ---------------------------------------------------

template <uint32_t dim_>
class Spatial
{
public:
	inline static constexpr uint32_t dim = dim_;
	using Vector = typename Mylib::Math::Vector<typename Mylib::Math::VectorStorage__<float, dim>>;
	using Point = Vector;

private:
	Spatial *anchor;

protected:
	MYLIB_OO_ENCAPSULATE_OBJ(Point, position)  // relative to the anchor

public:
	Spatial (Spatial *anchor_, const Point& position_)
		: anchor(anchor_),
		  position(position_)
	{
	}

	Spatial (const Point& position_)
		: anchor(nullptr),
		  position(position_)
	{
	}

	Point get_global_position () const
	{
		if (this->anchor) [[likely]] // most objects have an anchor
			return this->anchor->get_global_position() + this->position;
		else
			return this->position;
	}
};

using Spatial2D = Spatial<2>;
using Spatial3D = Spatial<3>;

// ---------------------------------------------------

class Scene : public Entity
{
public:
	Scene (const UserData& user_data_)
		: Entity(nullptr, user_data_)
	{
	}

	virtual void setup_render () = 0;
};

// ---------------------------------------------------

class Scene2D : public Scene
{
public:
	inline static constexpr uint32_t dim = 2;
	using Spatial = Game::Spatial<dim>;
	using Vector = Spatial::Vector;
	using Point = Spatial::Point;
	using UserData = Entity::UserData;

public:
	Scene2D (const UserData& user_data_)
		: Scene(user_data_)
	{
	}
};

// ---------------------------------------------------

template <uint32_t dim_>
class SpatialComponent : public Component, public Spatial<dim_>
{
public:
	inline static constexpr uint32_t dim = dim_;
	using Spatial = Game::Spatial<dim>;
	using Vector = Spatial::Vector;
	using Point = Spatial::Point;

public:
	SpatialComponent (Entity *parent_, Spatial *anchor_, const Point& position_)
		: Component(parent_),
		  Spatial(anchor_, position_)
	{
	}

	SpatialComponent (Entity *parent_, const Point& position_)
		: Component(parent_),
		  Spatial(position_)
	{
	}
};

using SpatialComponent2D = SpatialComponent<2>;
using SpatialComponent3D = SpatialComponent<3>;

// ---------------------------------------------------

template <uint32_t dim_>
class SpatialEntity : public Entity, public Spatial<dim_>
{
public:
	inline static constexpr uint32_t dim = dim_;
	using Spatial = Game::Spatial<dim>;
	using Vector = Spatial::Vector;
	using Point = Spatial::Point;
	using UserData = Entity::UserData;

public:
	SpatialEntity (Entity *parent_, const UserData& user_data_, Spatial *anchor_, const Point& position_)
		: Entity(parent_, user_data_),
		  Spatial(anchor_, position_)
	{
	}

	SpatialEntity (Entity *parent_, const UserData& user_data_, const Point& position_)
		: Entity(parent_, user_data_),
		  Spatial(position_)
	{
	}
};

using SpatialEntity2D = SpatialEntity<2>;
using SpatialEntity3D = SpatialEntity<3>;

// ---------------------------------------------------

template <uint32_t dim_>
class DynamicEntity : public SpatialEntity<dim_>
{
public:
	inline static constexpr uint32_t dim = dim_;
	using Spatial = Game::Spatial<dim>;
	using SpatialEntity = Game::SpatialEntity<dim>;
	using Vector = SpatialEntity::Vector;
	using Point = SpatialEntity::Point;
	using UserData = SpatialEntity::UserData;

protected:
	MYLIB_OO_ENCAPSULATE_OBJ_INIT_WITH_COPY_MOVE(Vector, velocity, Vector::zero())

public:
	DynamicEntity (Entity *parent_, const UserData& user_data_, Spatial *anchor_, const Point& position_, const Vector& velocity_)
		: SpatialEntity(parent_, user_data_, anchor_, position_),
		  velocity(velocity_)
	{
	}

	DynamicEntity (Entity *parent_, const UserData& user_data_, const Point& position_, const Vector& velocity_)
		: SpatialEntity(parent_, user_data_, position_),
		  velocity(velocity_)
	{
	}

	DynamicEntity (Entity *parent_, const UserData& user_data_, Spatial *anchor_, const Point& position_)
		: SpatialEntity(parent_, user_data_, anchor_, position_)
	{
	}

	DynamicEntity (Entity *parent_, const UserData& user_data_, const Point& position_)
		: SpatialEntity(parent_, user_data_, position_)
	{
	}

	void process_physics (const float dt) override
	{
		this->position += this->velocity * dt;
	}
};

using DynamicEntity2D = DynamicEntity<2>;
using DynamicEntity3D = DynamicEntity<3>;

// ---------------------------------------------------

class Main
{
public:
	struct InitConfig {
		const char *window_name;
		uint32_t window_width_px;
		uint32_t window_height_px;
		bool fullscreen;
		float target_dt;
		float max_dt;
		bool sleep_to_save_cpu;
		float sleep_threshold;
		bool busy_wait_to_ensure_fps;
	};

	enum class State {
		Initializing,
		Playing
	};

protected:
	const InitConfig config;
	MYLIB_OO_ENCAPSULATE_PTR(Scene*, scene)
	MYLIB_OO_ENCAPSULATE_SCALAR(bool, alive)
	MYLIB_OO_ENCAPSULATE_SCALAR_READONLY(State, state)

	MyGlib::Event::Quit::Descriptor event_quit_d;
	MyGlib::Event::KeyDown::Descriptor event_key_down_d;

private:
	static inline Main *instance = nullptr;

	Main (const InitConfig& config_, Scene *scene_);
	~Main ();

public:
	// delete default constructor
	Main () = delete;

	// delete copy/move constructor and assignment operator
	Main (const Main&) = delete;
	Main (Main&&) = delete;
	Main& operator= (const Main&) = delete;
	Main& operator= (Main&&) = delete;

	void run ();
	void event_quit (const MyGlib::Event::Quit::Type);
	void event_key_down_callback (const MyGlib::Event::KeyDown::Type& event);

	static inline Main& get () noexcept
	{
		return *instance;
	}

	static Main* load (const InitConfig& config_, Scene *scene_ = nullptr);
	static void unload ();
};

// ---------------------------------------------------

constexpr ClockDuration float_to_ClockDuration (const float t)
{
	return std::chrono::duration_cast<ClockDuration>(std::chrono::duration<float>(t));
}

constexpr float ClockDuration_to_float (const ClockDuration& d)
{
	return std::chrono::duration_cast<std::chrono::duration<float>>(d).count();
}

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif