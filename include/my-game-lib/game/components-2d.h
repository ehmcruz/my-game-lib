#ifndef __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__
#define __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__

#include <my-game-lib/game/game.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

class Rect2DRenderer : public SpatialComponent2D
{
public:
	using SpatialComponent = SpatialComponent2D;
	using Vector = SpatialComponent::Vector;
	using Point = SpatialComponent::Point;

private:
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Graphics::Rect2D, rect)
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Color, color)

public:
	Rect2DRenderer (Entity *parent_, SpatialComponent *anchor_, const Point& position_, const Vector& size_, const Color& color_)
		: Component(parent_),
		  SpatialComponent2D(parent_, anchor_, position_),
		  rect(size_),
		  color(color_)
	{
	}

	void process_render (const float dt) override final;
};

// ---------------------------------------------------

class Sprite2DRenderer : public SpatialComponent2D
{
public:
	using SpatialComponent = SpatialComponent2D;
	using Vector = SpatialComponent::Vector;
	using Point = SpatialComponent::Point;

private:
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Graphics::Rect2D, rect)
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(TextureDescriptor, texture)

public:
	Sprite2DRenderer (Entity *parent_, SpatialComponent *anchor_, const Point& position_, const Vector& size_, const TextureDescriptor& texture_)
		: Component(parent_),
		  SpatialComponent2D(parent_, anchor_, position_),
		  rect(size_),
		  texture(texture_)
	{
	}

	void process_render (const float dt) override final;
};

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif