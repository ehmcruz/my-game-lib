#ifndef __MY_GAME_LIB_TEXTURE_ATLAS_HEADER_H__
#define __MY_GAME_LIB_TEXTURE_ATLAS_HEADER_H__

#include <string_view>
#include <vector>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>

#include <my-game-lib/graphics.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

class TextureAtlas
{
protected:
	struct EmptyArea {
		int32_t x_ini;
		int32_t y_ini;
		int32_t x_end;
		int32_t y_end;
	};

	const int32_t min_size;
	const int32_t max_size;

	std::vector<EmptyArea> empty_areas;
	std::vector<TextureDescriptor> textures;

public:
	TextureAtlas (const int32_t min_size_, const int32_t max_size_);

	MYLIB_DELETE_COPY_MOVE_CONSTRUCTOR_ASSIGN(TextureAtlas)

	void add_texture (const TextureDescriptor& texture);
	bool process ();

protected:
	EmptyArea* find_empty_area (const TextureDescriptor& texture);
};

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib

#endif