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

#include <boost/static_string.hpp>

#include <my-game-lib/my-game-lib.h>
#include <my-game-lib/graphics.h>
#include <my-game-lib/events.h>
#include <my-game-lib/exception.h>

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

inline Mylib::Memory::Manager *memory_manager = &Mylib::Memory::default_manager;

inline std::mt19937_64 random_generator;

inline auto timer = Mylib::Event::make_timer<Coroutine>(Clock::now);
using Timer = decltype(timer);

inline Mylib::InterpolationManager<Coroutine, float> interpolation_manager;

// ---------------------------------------------------

template <typename T, typename... Types>
[[nodiscard]] unique_ptr<T> make_unique (Types&&... vars)
{
	return Mylib::Memory::make_unique<T>(*memory_manager, std::forward<Types>(vars)...);
}

// ---------------------------------------------------

class Component
{
protected:
	MYLIB_OO_ENCAPSULATE_PTR_INIT(Component*, parent, nullptr)
	MYLIB_OO_ENCAPSULATE_OBJ(boost::static_string<16>, name)

public:
	Component () = default;

	virtual ~Component () { };

	// called after all the components have been processed and rendered
	virtual void frame_finished () { };
};

// ---------------------------------------------------

class UpdateInterface
{
public:
	// called for each update frame
	virtual void process_update (const float dt) = 0;
};

// ---------------------------------------------------

class PhysicsInterface
{
public:
	// called for each update frame
	virtual void process_physics (const float dt) = 0;
};

// ---------------------------------------------------

class RenderInterface
{
public:
	// called for each rendered frame
	virtual void process_render (const float dt) = 0;
};

// ---------------------------------------------------

template <uint32_t dim_>
class TransformInterface
{
public:
	inline static constexpr uint32_t dim = dim_;
	using Vector = typename Mylib::Math::Vector<typename Mylib::Math::VectorStorage__<float, dim>>;
	using Point = Vector;

protected:
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Vector, position)

public:
	TransformInterface (const Point& position_)
		: position(position_)
	{
	}

	void set_position (const float x, const float y) noexcept
	{
		this->position.x = x;
		this->position.y = y;
	}
};

// ---------------------------------------------------

template <uint32_t dim_>
class TransformComponent : public Component, public TransformInterface<dim_>
{
public:
	inline static constexpr uint32_t dim = dim_;
	using TransformInterface = Game::TransformInterface<dim>;
	using Vector = TransformInterface::Vector;
	using Point = TransformInterface::Point;

public:
	TransformComponent (const Point& position_)
		: Component(),
		  TransformInterface(position_)
	{
	}

	Point get_global_position () const
	{
		if (this->parent != nullptr) [[likely]]
			return this->position + static_cast<TransformComponent*>(this->parent)->get_global_position();
		else
			return this->position;
	}
};

using TransformComponent2D = TransformComponent<2>;
using TransformComponent3D = TransformComponent<3>;

// ---------------------------------------------------

class ComponentWithParentException : public Exception
{
private:
	Component *component = nullptr;

public:
	ComponentWithParentException (const std::source_location& location_, const char *assert_str_, const char *extra_msg_, Component *component_)
		: Exception(location_, assert_str_, extra_msg_),
		  component(component_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Component with parent exception" << std::endl
			<< "Component name: " << this->component->get_ref_name() << std::endl;
	}
};

// ---------------------------------------------------

template <uint32_t dim_>
class Entity : public TransformComponent<dim_>
{
public:
	inline static constexpr uint32_t dim = dim_;
	using TransformComponent = Game::TransformComponent<dim>;
	using Vector = TransformComponent::Vector;
	using Point = TransformComponent::Point;
	using UserData = uint64_t;

public:
	UserData user_data;

protected:
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<unique_ptr<Component>>, components)
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<unique_ptr<Entity>>, entities)
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<UpdateInterface*>, updatables)
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<PhysicsInterface*>, physicsables)
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<RenderInterface*>, renderables)

public:
	Entity (const Point& position_, const UserData& user_data_)
		: TransformComponent(position_),
		  user_data(user_data_)
	{
	}

	template <typename T>
	void add_child (unique_ptr<T> child)
	{
		static_assert(std::is_base_of_v<Component, T>);
		constexpr uint32_t other_dim = (dim == 2) ? 3 : 2;

		T *child_ptr = child.get();

		mylib_assert_exception_args(child->get_parent() == nullptr, ComponentWithParentException, child_ptr)

		if constexpr (std::is_base_of_v<Entity<dim>, T>)
			this->entities.push_back(std::move(child));
		else if constexpr (std::is_base_of_v<Entity<other_dim>, T>)
			static_assert(0, "Cannot add a child of a different dimension");
		else if constexpr (std::is_base_of_v<Component, T>)
			this->components.push_back(std::move(child));
		else
			static_assert(0, "Cannot add a child of this type");
		
		if constexpr (std::is_base_of_v<UpdateInterface, T>)
			this->updatables.push_back(child_ptr);
		if constexpr (std::is_base_of_v<PhysicsInterface, T>)
			this->physicsables.push_back(child_ptr);
		if constexpr (std::is_base_of_v<RenderInterface, T>)
			this->renderables.push_back(child_ptr);
		
		child_ptr->set_parent(this);
	}

	void loop_update (const float dt)
	{
		for (auto& entity : this->entities)
			entity->loop_update(dt);
		for (auto& component : this->updatables)
			component->process_update(dt);
	}

	void loop_physics (const float dt)
	{
		for (auto& entity : this->entities)
			entity->loop_physics(dt);
		for (auto& component : this->physicsables)
			component->process_physics(dt);
	}

	void loop_render (const float dt)
	{
		for (auto& entity : this->entities)
			entity->loop_render(dt);
		for (auto& component : this->renderables)
			component->process_render(dt);
	}
};

using Entity2D = Entity<2>;
using Entity3D = Entity<3>;

// ---------------------------------------------------

class Scene
{
public:
	virtual void process (const float dt) = 0;
	virtual void setup_render () = 0;
};

// ---------------------------------------------------

template <uint32_t dim_>
class Scene__ : public Entity<dim_>, public Scene
{
public:
	inline static constexpr uint32_t dim = dim_;
	using Entity = Game::Entity<dim>;
	using Vector = Entity::Vector;
	using Point = Entity::Point;
	using UserData = Entity::UserData;

public:
	Scene__ (const UserData& user_data_)
		: Entity(Vector::zero(), user_data_),
		  Scene()
	{
	}

	void process (const float dt) override final
	{
		this->loop_update(dt);

		if (UpdateInterface *component = dynamic_cast<UpdateInterface*>(this))
			component->process_update(dt);

		this->loop_physics(dt);

		if (PhysicsInterface *component = dynamic_cast<PhysicsInterface*>(this))
			component->process_physics(dt);

		this->setup_render();

		this->loop_render(dt);

		if (RenderInterface *component = dynamic_cast<RenderInterface*>(this))
			component->process_render(dt);
	}
};

// ---------------------------------------------------

class Scene2D : public Scene__<2>
{
public:
	inline static constexpr uint32_t dim = 2;
	using Scene = Game::Scene__<dim>;
	using Vector = Scene::Vector;
	using Point = Scene::Point;
	using UserData = Scene::UserData;

public:
	Scene2D (const UserData& user_data_)
		: Scene(user_data_)
	{
	}

	void setup_render () override final;
	virtual MyGlib::Graphics::RenderArgs2D setup_render_args () = 0;
};


// ---------------------------------------------------

template <uint32_t dim_>
class DynamicEntity : public Entity<dim_>, public PhysicsInterface
{
public:
	inline static constexpr uint32_t dim = dim_;
	using Entity = Game::Entity<dim>;
	using Vector = Entity::Vector;
	using Point = Entity::Point;
	using UserData = Entity::UserData;

protected:
	MYLIB_OO_ENCAPSULATE_OBJ_INIT_WITH_COPY_MOVE(Vector, velocity, Vector::zero())

public:
	DynamicEntity (const Point& position_, const UserData& user_data_, const Vector& velocity_)
		: Entity(position_, user_data_),
		  velocity(velocity_)
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