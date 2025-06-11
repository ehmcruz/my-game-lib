#include <cstdint>

#include <utility>
#include <type_traits>
#include <unordered_map>
#include <algorithm>

#include <boost/functional/hash.hpp>

#include <my-game-lib/game/game.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

template <uint32_t dim_>
class Entity;

// ---------------------------------------------------

template <uint32_t dim_>
class RectCollider__ : public TransformComponent<dim_>, public ColliderInterface<dim_>
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
};

using Rect2DCollider = RectCollider__<2>;
using RectCube3DCollider = RectCollider__<3>;

// ---------------------------------------------------

// Returns a pair where:
// 1st element: true if there is a collision, false otherwise
// 2nd element: the collision resolution vector (how much obj_b should move to resolve the collision)

template <uint32_t dim>
std::pair<bool, Vector<dim>> check_collision (const RectCollider__<dim>& collider_a,
										      const RectCollider__<dim>& collider_b) noexcept
{
	using Vector = Game::Vector<dim>;
	Vector collision_vector;

	const auto& transform_a = collider_a.get_transform();
	const auto& transform_b = collider_b.get_transform();

	const Vector position_a = (transform_a * Game::Vector<dim+1>(Vector::zero(), 1.0f)).template to_reduced<Vector>();
	const Vector position_b = (transform_b * Game::Vector<dim+1>(Vector::zero(), 1.0f)).template to_reduced<Vector>();

	const Vector distance = position_b - position_a;
	const Vector target_distance = (collider_a.get_ref_size() + collider_b.get_ref_size()) / 2.0f;

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

// ---------------------------------------------------

template <uint32_t dim_>
class CollisionManager
{
public:
	inline static constexpr uint32_t dim = dim_;
	using Entity = Game::Entity<dim>;
	using ColliderInterface = Game::ColliderInterface<dim>;
	using RectCollider = Game::RectCollider__<dim>;

private:
	struct ActiveCollision {
		ColliderInterface& collider_a;
		ColliderInterface& collider_b;

		ActiveCollision (ColliderInterface& collider_a_, ColliderInterface& collider_b_)
			: collider_a(collider_a_),
			  collider_b(collider_b_)
		{
		}
	};

	using Key = std::pair<const ColliderInterface*, const ColliderInterface*>;

	static Key make_key (const ColliderInterface& a, const ColliderInterface& b) noexcept
	{
		if (&a < &b)
			return std::make_pair(&a, &b);
		else
			return std::make_pair(&b, &a);
	}

	static Key make_key (const ActiveCollision& collision) noexcept
	{
		return make_key(collision.collider_a, collision.collider_b);
	}

	Entity& owner;
	std::unordered_map<Key, ActiveCollision, boost::hash<Key>> active_collisions;

public:
	CollisionManager (Entity& owner_)
		: owner(owner_)
	{
	}

private:
	void process_collision (Entity& entity_a, const ColliderInterface *collider_a, Entity& entity_b, const ColliderInterface *collider_b)
	{
		if (const auto *rect_a = dynamic_cast<const RectCollider*>(collider_a)) {
			if (const auto *rect_b = dynamic_cast<const RectCollider*>(collider_b)) {
				const auto [collides, resolution_vector] = check_collision(*rect_a, *rect_b);

				if (collides) {
					// Handle collision resolution here
					// For example, move entity_b by resolution_vector
				}
			}
		}
	}

	void process_collision (Entity& entity_a, Entity& sub_entity_a, Entity& entity_b, Entity& sub_entity_b)
	{
		for (auto& collider_a : sub_entity_a.get_ref_colliders()) {
			for (auto& collider_b : sub_entity_b.get_ref_colliders())
				this->process_collision(entity_a, collider_a, entity_b, collider_b);
		}
	}

	void process_collision (Entity& entity_a, Entity& entity_b)
	{
		for (auto& sub_entity_a : entity_a.get_ref_entities()) {
			for (auto& sub_entity_b : entity_b.get_ref_entities()) {
				this->process_collision(entity_a, *sub_entity_a, entity_b, *sub_entity_b);
				this->process_collision(entity_a, entity_a, entity_b, *sub_entity_b);
			}
			this->process_collision(entity_a, *sub_entity_a, entity_b, entity_b);
		}
		this->process_collision(entity_a, entity_a, entity_b, entity_b);
	}

public:
	void process_collisions (const float dt)
	{
		auto& entities = this->owner.get_ref_entities();

		for (auto it_a = entities.begin(); it_a != entities.end(); it_a++) {
			Entity& entity_a = **it_a;

			for (auto it_b = std::next(it_a); it_b != entities.end(); it_b++) {
				Entity& entity_b = **it_b;
				this->process_collision(entity_a, entity_b);
			}
		}
	}
};

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib
