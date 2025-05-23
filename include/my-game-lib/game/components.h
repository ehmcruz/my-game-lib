#ifndef __MY_GAME_LIB_GAME_COMPONENTS_HEADER_H__
#define __MY_GAME_LIB_GAME_COMPONENTS_HEADER_H__

#include <my-game-lib/game/game.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

template <uint32_t dim_>
class RectCollider__ : public TransformComponent<dim_>
{
public:
	inline static constexpr uint32_t dim = dim_;
	using TransformComponent = Game::TransformComponent<dim_>;
	using Vector = TransformComponent::Vector;
	using Point = TransformComponent::Point;
	
protected:
	MYLIB_OO_ENCAPSULATE_OBJ(Vector, size)

public:
	RectCollider__ (const Vector& size_)
		: TransformComponent(),
		  size(size_)
	{
	}

	// Returns a pair where:
	// 1st element: true if there is a collision, false otherwise
	// 2nd element: the collision resolution vector (how much obj_b should move to resolve the collision)

	static std::pair<bool, Vector> check_collision (const RectCollider__& collider_a,
													const RectCollider__& collider_b) noexcept
	{
		Vector collision_vector;
	
		const Vector distance = collider_b.get_global_position() - collider_a.get_global_position();
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

using Rect2DCollider = RectCollider__<2>;
using RectCube3DCollider = RectCollider__<3>;

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif