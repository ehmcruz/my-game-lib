#ifndef __MY_GAME_LIB_TEXTURE_ATLAS_HEADER_H__
#define __MY_GAME_LIB_TEXTURE_ATLAS_HEADER_H__

#include <string_view>
#include <vector>
#include <list>

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

class TextureAtlasCreator
{
public:
	struct AtlasTexture {
		TextureInfo *texture;
		int32_t x_ini;
		int32_t y_ini;
	};

protected:
	std::list<TextureInfo*> textures;

public:
	TextureAtlasCreator () = default;
	MYLIB_DELETE_COPY_MOVE_CONSTRUCTOR_ASSIGN(TextureAtlasCreator)

	void add_texture (TextureInfo& texture);
	std::vector<AtlasTexture> create_atlas (const int32_t atlas_size);
};

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib

#endif