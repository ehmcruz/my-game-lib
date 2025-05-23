#ifndef __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__
#define __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__

#include <my-game-lib/game/game.h>

#include <my-lib/matrix.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

class Rect2DRenderer : public TransformComponent2D, public RenderInterface
{
public:
	using TransformComponent = TransformComponent2D;
	using Vector = TransformComponent::Vector;
	using Point = TransformComponent::Point;

private:
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Graphics::Rect2D, rect)
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Color, color)

public:
	Rect2DRenderer (const Vector& size_, const Color& color_)
		: TransformComponent2D(),
		  rect(size_),
		  color(color_)
	{
	}

	void process_render (const float dt) override final;
};

// ---------------------------------------------------

class Sprite2DRenderer : public TransformComponent2D, public RenderInterface
{
public:
	using TransformComponent = TransformComponent2D;
	using Vector = TransformComponent::Vector;
	using Point = TransformComponent::Point;

private:
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Graphics::Rect2D, rect)
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(TextureDescriptor, texture)

public:
	Sprite2DRenderer (const Vector& size_, const TextureDescriptor& texture_)
		: TransformComponent2D(),
		  rect(size_),
		  texture(texture_)
	{
	}

	void process_render (const float dt) override final;
};

// ---------------------------------------------------

class TileMap : public Entity2D
{
public:
	using Entity = Game::Entity2D;
	using Vector = Entity2D::Vector;
	using Point = Entity2D::Point;

protected:
	Mylib::Matrix<Entity*> matrix;
	Vector tile_size;

protected:
	using Entity2D::add_child;

public:
	TileMap (const UserData& user_data_, const uint32_t nrows, const uint32_t ncols, const Vector& tile_size_)
		: Entity(user_data_),
		  matrix(nrows, ncols, nullptr),
		  tile_size(tile_size_)
	{
	}

	template <typename T>
	void set (const uint32_t row, const uint32_t col, unique_ptr<T> entity)
		requires (std::is_base_of_v<Entity, T>)
	{
		entity->set_position((static_cast<float>(row) + 0.5f) * this->tile_size.x,
							(static_cast<float>(col) + 0.5f) * this->tile_size.y);
		this->matrix[row, col] = entity.get();
		this->add_child(std::move(entity));
	}

	void set (const uint32_t row, const uint32_t col, unique_ptr<Sprite2DRenderer> sprite);
	void set (const uint32_t row, const uint32_t col, TextureDescriptor texture);

	Entity& operator[] (const uint32_t row, const uint32_t col)
	{
		return *this->matrix[row, col];
	}

	const Entity& operator[] (const uint32_t row, const uint32_t col) const
	{
		return *this->matrix[row, col];
	}
};

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif