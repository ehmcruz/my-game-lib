#include <my-game-lib/texture-atlas.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

TextureAtlas::TextureAtlas (const int32_t max_size_) :
	max_size(max_size_)
{
	this->empty_areas.push_back({0, 0, max_size, max_size});
}

// ---------------------------------------------------

void TextureAtlas::add_texture (const TextureDescriptor& texture)
{
	this->textures.push_back(texture);
}

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib