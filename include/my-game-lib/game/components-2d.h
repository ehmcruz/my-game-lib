#ifndef __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__
#define __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__

#include <string_view>

#include <my-game-lib/game/game.h>
#include <my-game-lib/exception.h>

#include <my-lib/matrix.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

class Rect2DRenderer : public TransformComponent2D, public MovableInterface2D, public RenderInterface
{
public:
	using TransformComponent = TransformComponent2D;
	using Vector = TransformComponent::Vector;
	using Point = TransformComponent::Point;

private:
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Vector, size)
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Color, color)
	MYLIB_OO_ENCAPSULATE_SCALAR_INIT(float, z, 0.0f)

public:
	Rect2DRenderer (const Vector& size_, const Color& color_)
		: TransformComponent2D(),
		  MovableInterface2D(this),
		  size(size_),
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
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Vector, size)
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(TextureDescriptor, texture)
	MYLIB_OO_ENCAPSULATE_SCALAR_INIT(float, z, 0.0f)

public:
	Sprite2DRenderer (const Vector& size_, const TextureDescriptor& texture_)
		: TransformComponent2D(),
		  size(size_),
		  texture(texture_)
	{
	}

	void process_render (const float dt) override final;
};

// ---------------------------------------------------

class TileMap : public Entity2D
{
public:
	using TransformComponent = Game::TransformComponent2D;
	using Entity = Game::Entity2D;
	using Vector = Entity2D::Vector;
	using Point = Entity2D::Point;

protected:
	Mylib::Matrix<TransformComponent*> matrix;
	Vector tile_size;
	std::vector<TextureDescriptor> textures;

protected:
	using Entity2D::add_child;

public:
	TileMap (const uint32_t nrows, const uint32_t ncols, const Vector& tile_size_)
		: Entity(),
		  matrix(nrows, ncols, nullptr),
		  tile_size(tile_size_)
	{
	}

	// Tiled TMX interface
	TileMap (const std::string_view tmx_fname, const std::string_view layer_name, const Vector& tile_size_);

	template <typename T>
	void set (const uint32_t row, const uint32_t col, unique_ptr<T> component)
		requires (std::is_base_of_v<TransformComponent, T>)
	{
		component->set_translation((static_cast<float>(row) + 0.5f) * this->tile_size.x,
							(static_cast<float>(col) + 0.5f) * this->tile_size.y);
		this->matrix[row, col] = component.get();
		this->add_child(std::move(component));
	}

	void set (const uint32_t row, const uint32_t col, TextureDescriptor texture);

	TransformComponent& get (const uint32_t row, const uint32_t col)
	{
		return *this->matrix[row, col];
	}

	const TransformComponent& get (const uint32_t row, const uint32_t col) const
	{
		return *this->matrix[row, col];
	}
};

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif