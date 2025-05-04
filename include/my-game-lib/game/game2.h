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

#include <cmath>
#include <cstdlib>

#include <my-game-lib/graphics.h>

#include <my-lib/std.h>
#include <my-lib/macros.h>

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

// ---------------------------------------------------

class Entity;

class Component
{
protected:
	MYLIB_OO_ENCAPSULATE_PTR(Entity*, parent)

public:
	// called for each rendered frame
	virtual void process_render (const float dt) { };

	// called for each physics frame, before process_physics
	virtual void process_update (const float dt) { };

	// called for each physics frame
	virtual void process_physics (const float dt) { };
};

// ---------------------------------------------------

template <uint32_t dim_>
class SpatialComponent : virtual public Component
{
public:
	static constexpr uint32_t dim = dim_;
	using Vector = typename Mylib::Math::Vector<typename Mylib::Math::VectorStorage__<float, dim>>;
	using Point = Vector;
	using SpatialEntity = typename Game::SpatialEntity<dim>;

private:
	SpatialEntity *position_anchor;

protected:
	MYLIB_OO_ENCAPSULATE_OBJ(Point, position)  // relative to the position_anchor entity

public:
	Point get_global_position () const
	{
		if (this->position_anchor) [[likely]] // most objects have an anchor
			return this->position_anchor->get_position() + this->position;
		else
			return this->position;
	}
};

using SpatialComponent2D = SpatialComponent2d<2>;
using SpatialComponent3D = SpatialComponent2d<3>;

// ---------------------------------------------------

template <uint32_t dim_>
class RectCollider__ : public SpatialComponent<dim_>
{
public:
	static constexpr uint32_t dim = dim_;
	
protected:
	MYLIB_OO_ENCAPSULATE_OBJ(Vector, size)

public:
	// Returns a pair where:
	// 1st element: true if there is a collision, false otherwise
	// 2nd element: the collision resolution vector (how much obj_b should move to resolve the collision)

	static std::pair<bool, Vector> check_collision (const RectCollider& collider_a,
	                                                const Point& owner_pos_a,
													const RectCollider& collider_b,
	                                                const Point& owner_pos_b) noexcept
	{
		Vector collision_vector;
	
		const Vector distance = owner_b_pos - owner_a_pos;
		const Vector target_distance = (collider_a.size + collider_b.size) / 2.0f;
	
		/*
			Let's think a bit.
			First, we only care about a valid collision_vector
			if the colliders are colliding.
			If they are not colliding, we don't care if the collision_vector
			calculated is valid or not.
		*/
	
		/*
			x axis is in the right direction in the plane.
			If b is right of a, then the distance.x is positive.
			Since the collider returns what we should do with object b
			to solve the collision, collision_vector.x should be positive.
			Otherwise, it should be negative.
		*/
	
		const bool colliding_x = std::abs(distance.x) < target_distance.x;
		collision_vector.x = std::copysign(target_distance.x - std::abs(distance.x), distance.x);
	
		/*
			y axis is in the up direction in the screen.
			If b is above a, then the distance.y is positive.
			Since the collider returns what we should do with object b
			to solve the collision, collision_vector.y should be positive.
			Otherwise, it should be negative.
		*/
	
		const bool colliding_y = std::abs(distance.y) < target_distance.y;
		collision_vector.y = std::copysign(target_distance.y - std::abs(distance.y), distance.y);
	
		/*
			z axis represents the altitude.
			The higher the altitude, the higher the z value, since we use right-handed coordinates.
			If b is above a, then the distance.y is positive.
			Since the collider returns what we should do with object b
			to solve the collision, collision_vector.y should be positive.
			Otherwise, it should be negative.
		*/
	
		if constexpr (dim == 3) {
			const bool colliding_z = std::abs(distance.z) < target_distance.z;
			collision_vector.z = std::copysign(target_distance.z - std::abs(distance.z), distance.z);
	
			return std::pair<bool, Vector>(colliding_x & colliding_y & colliding_z, collision_vector);
		}
		else
			return std::pair<bool, Vector>(colliding_x & colliding_y, collision_vector);
	}
};

using RectCollider2D = RectCollider__<2>;
using RectCollider3D = RectCollider__<3>;

// ---------------------------------------------------

class Entity : virtual public Component
{
public:
	uint64_t user_data;

protected:
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<unique_ptr<Component>>, components)
	MYLIB_OO_ENCAPSULATE_OBJ(std::list<unique_ptr<Entity>>, entities)

public:
	template <typename T>
	void add_child (unique_ptr<T> child)
	{
		if constexpr (std::is_base_of_v<Entity, T>)
			this->components.push_back(std::move(child));
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
class SpatialEntity :
	public Entity,
	public SpatialComponent<dim_>
{
public:
	static constexpr uint32_t dim = dim_;


protected:

};

// ---------------------------------------------------

template <uint32_t dim_>
class DynamicEntity : public SpatialEntity<dim_>
{
protected:
	MYLIB_OO_ENCAPSULATE_OBJ(Vector, velocity)

public:
	void process_physics (const float dt) override
	{
		this->position += this->velocity * dt;
	}
};

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif