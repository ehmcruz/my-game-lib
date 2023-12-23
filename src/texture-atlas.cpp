#include <algorithm>
#include <limits>

#include <my-game-lib/texture-atlas.h>
#include <my-game-lib/debug.h>

#include <my-lib/math.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

void TextureAtlasCreator::add_texture (TextureDescriptor& texture)
{
	this->textures.push_back(&texture);
}

// ---------------------------------------------------

std::vector<TextureDescriptor> TextureAtlasCreator::create_atlas (const int32_t atlas_size)
{
	std::vector<TextureDescriptor> atlas;

	if (this->textures.empty())
		return atlas;

	// sort textures by area
	this->textures.sort([](const TextureDescriptor *a, const TextureDescriptor *b) -> bool {
		return (a->width_px * a->height_px) > (b->width_px * b->height_px);
	});

	std::list<EmptyArea> empty_areas;

	for (auto *tex_desc : this->textures)
		mylib_assert_exception_msg((tex_desc->width_px <= atlas_size) && (tex_desc->height_px <= atlas_size), "A texture size is bigger than max size.")

	empty_areas.push_back( EmptyArea {
		.x_ini = 0,
		.y_ini = 0,
		.x_end = atlas_size,
		.y_end = atlas_size
		});

//	for (auto& tex_desc : this->textures)
//		dprintln("Area ", (tex_desc.width_px * tex_desc.height_px), " ", tex_desc.width_px, "x", tex_desc.height_px);

	decltype(this->textures)::iterator next_it;
	for (auto it = this->textures.begin(); it != this->textures.end(); it = next_it) {
		next_it = it;
		next_it++; // list doesn't have operator+ overload

		auto *tex_desc_ptr = *it;
		auto& tex_desc = *tex_desc_ptr;

		auto empty_area_it = this->find_empty_area(empty_areas, tex_desc);

		if (empty_area_it == empty_areas.end()) // no space for the texture, try next one
			continue;
		
		auto& empty_area = *empty_area_it;

		// We have found an empty area that fits the texture.
		
		// set the position of the texture in the atlas
		tex_desc.pos_x_px = empty_area.x_ini;
		tex_desc.pos_y_px = empty_area.y_ini;

		// add texture to atlas
		atlas.push_back(tex_desc);

		//dprintln("Texture of size", tex_desc.width_px, "x", tex_desc.height_px, " allocated at position ", tex_desc.pos_x_px, "x", tex_desc.pos_y_px);

		// update empty space

		if (tex_desc.width_px == (empty_area.x_end - empty_area.x_ini)) {
			// The texture fills the empty area horizontally.
			// We have to check if the texture also fills the empty area vertically.

			if (tex_desc.height_px == (empty_area.y_end - empty_area.y_ini)) {
				// The texture also fills the empty area vertically, so we can remove it from the empty areas list.
				empty_areas.erase(empty_area_it);
			}
			else {
				// The texture doesn't fill the empty area vertically, so we have to update the empty area.
				empty_area.y_ini += tex_desc.height_px;
			}
		}
		else {
			// The texture doesn't fill the empty area horizontally.

			if (tex_desc.height_px == (empty_area.y_end - empty_area.y_ini)) {
				// The texture fills the empty area vertically, so we can remove it from the empty areas list.
				empty_area.x_ini += tex_desc.width_px;
			} else {
				// The texture doesn't fill the empty area vertically, so we have to update the empty area.
				// It also doesn't fill the empty area horizontally, so we have to add a new empty area.

				// add empty area below the texture
				empty_areas.push_back( EmptyArea {
					.x_ini = empty_area.x_ini,
					.y_ini = empty_area.y_ini + tex_desc.height_px,
					.x_end = empty_area.x_ini + tex_desc.width_px,
					.y_end = empty_area.y_end
					});
				
				// update empty area to the right of the texture
				empty_area.x_ini += tex_desc.width_px;
			}
		}

		// now that we found a place for the texture, we can remove it from the list
		this->textures.erase(it);
	}

	return atlas;
}

// ---------------------------------------------------

std::list<TextureAtlasCreator::EmptyArea>::iterator TextureAtlasCreator::find_empty_area (std::list<EmptyArea>& empty_areas, const TextureDescriptor& texture)
{
	std::list<EmptyArea>::iterator best_it = empty_areas.end();
	int32_t best_area = std::numeric_limits<int32_t>::max();

	for (auto it = empty_areas.begin(); it != empty_areas.end(); it++) {
		auto& empty_area = *it;
		const int32_t w = empty_area.x_end - empty_area.x_ini;
		const int32_t h = empty_area.y_end - empty_area.y_ini;
		const int32_t area = w * h;

		if (w >= texture.width_px && h >= texture.height_px) {
			// If we arrive here, we have found an empty area that fits the texture.
			// Now, let's check if it's the best empty area.

			if (area < best_area) {
				best_it = it;
				best_area = area;
			}
		}
	}

	return best_it; // end() if no empty area fits the texture
}

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib