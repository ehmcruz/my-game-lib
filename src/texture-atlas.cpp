#include <algorithm>

#include <my-game-lib/texture-atlas.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

TextureAtlas::TextureAtlas (const int32_t min_size_, const int32_t max_size_)
	: min_size(min_size_), max_size(max_size_)
{
	this->empty_areas.push_back({0, 0, min_size, min_size});
}

// ---------------------------------------------------

void TextureAtlas::add_texture (const TextureDescriptor& texture)
{
	this->textures.push_back(texture);
}

// ---------------------------------------------------

bool TextureAtlas::process ()
{
	// sort textures by area
	std::sort(this->textures.begin(), this->textures.end(), [](const TextureDescriptor& a, const TextureDescriptor& b) -> bool {
		return (a.width_px * a.height_px) > (b.width_px * b.height_px);
	});

//	for (auto& tex_desc : this->textures)
//		std::cout << "Area " << (tex_desc.width_px * tex_desc.height_px) << " " << tex_desc.width_px << "x" << tex_desc.height_px << std::endl;

	for (auto& tex_desc : this->textures) {

	}

//exit(1);
}

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib