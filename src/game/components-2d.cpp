#include <filesystem>
#include <sstream>
#include <algorithm>
#include <cctype>

#include <my-game-lib/game/game.h>
#include <my-game-lib/game/components-2d.h>
#include <my-game-lib/graphics.h>
#include <my-game-lib/opengl/opengl.h>

#include <pugixml.hpp>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

using Graphics::Opengl::Opengl_TextureDescriptor;

namespace Enums {
	// 6 vertices counter-clockwise
	enum Rect2DVertexPositionIndex {
		RightBottom,
		RightTop,
		LeftTop,
		RightBottomAgain,
		LeftTopAgain,
		LeftBottom
	};
}

// ---------------------------------------------------

static std::array<Vector2, 6> generate_local_vertices_rect2d (const Vector2 size)
{
	const auto hs = size * 0.5f; // half size
	std::array<Vector2, 6> local_vertices;

	using enum Enums::Rect2DVertexPositionIndex;

	// draw counter-clockwise

	// first triangle
	local_vertices[RightBottom].set(hs.x, -hs.y);
	local_vertices[RightTop].set(hs.x, hs.y);
	local_vertices[LeftTop].set(-hs.x, hs.y);

	// second triangle
	local_vertices[RightBottomAgain].set(hs.x, -hs.y);
	local_vertices[LeftTopAgain].set(-hs.x, hs.y);
	local_vertices[LeftBottom].set(-hs.x, -hs.y);

	return local_vertices;
}

// ---------------------------------------------------

void Rect2DRenderer::process_render (const float dt)
{
	this->process_movable(dt);

	auto *renderer = static_cast<Graphics::Opengl::Renderer*>(Game::renderer);
	auto& program = *renderer->get_program_triangle_color();

	const Matrix3& transform = this->get_global_transform();
	constexpr uint32_t n_vertices = 6;
	
#if 0
	dprint( "local_pos:" )
	Mylib::Math::println(world_pos);

	dprint( "clip_pos:" )
	Mylib::Math::println(clip_pos);
//exit(1);
#endif

	std::array<Vector2, n_vertices> local_vertices = generate_local_vertices_rect2d(this->size);

	auto vertices = program.alloc_vertices(n_vertices);

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex.pos = transform * Vector3(local_vertices[i], 1);
		vertices[i].gvertex.pos.z = this->z;
		vertices[i].offset.set_zero();
		vertices[i].color = this->color;
	}
}

// ---------------------------------------------------

void Sprite2DRenderer::process_render (const float dt)
{
	auto *renderer = static_cast<Graphics::Opengl::Renderer*>(Game::renderer);
	auto& program = *renderer->get_program_triangle_texture();

	const Matrix3 transform = this->get_global_transform();
	constexpr uint32_t n_vertices = 6;
	
	using Rect2DVertexPositionIndex = Enums::Rect2DVertexPositionIndex;
	using TextureVertexPositionIndex = Graphics::Enums::TextureVertexPositionIndex;

	std::array<Vector2, n_vertices> local_vertices = generate_local_vertices_rect2d(this->size);
	const Opengl_TextureDescriptor *desc = this->texture.info->data.get_value<Opengl_TextureDescriptor*>();

	auto vertices = program.alloc_vertices(n_vertices);

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex.pos = transform * Vector3(local_vertices[i], 1);
		vertices[i].gvertex.pos.z = this->z;
		vertices[i].offset.set_zero();
	}

	vertices[Rect2DVertexPositionIndex::RightBottom].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::RightBottom].x, desc->tex_coords[TextureVertexPositionIndex::RightBottom].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::RightTop].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::RightTop].x, desc->tex_coords[TextureVertexPositionIndex::RightTop].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::LeftTop].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::LeftTop].x, desc->tex_coords[TextureVertexPositionIndex::LeftTop].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::RightBottomAgain].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::RightBottom].x, desc->tex_coords[TextureVertexPositionIndex::RightBottom].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::LeftTopAgain].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::LeftTop].x, desc->tex_coords[TextureVertexPositionIndex::LeftTop].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::LeftBottom].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::LeftBottom].x, desc->tex_coords[TextureVertexPositionIndex::LeftBottom].y, desc->atlas->texture_depth);
}

// ---------------------------------------------------

TileMap::TileMap (const std::string_view tmx_fname, const std::string_view layer_name, const Vector& tile_size_)
	: Entity(),
	  tile_size(tile_size_)
{
	std::unordered_map<uint32_t, TextureDescriptor> tile_textures;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(tmx_fname.data());

	mylib_assert_exception_msg_args(result, FileException, "Error loading TMX file.", tmx_fname)

	dprintln("TMX file loaded: ", tmx_fname);

	pugi::xml_node map_node = doc.child("map");
	mylib_assert_exception_msg_args(map_node, FileException, "Map node does not exist.", tmx_fname)

	const uint32_t cols  = map_node.attribute("width").as_uint();
	const uint32_t rows = map_node.attribute("height").as_uint();

	dprintln("TileMap size rows: ", rows, " cols: ", cols);

	this->matrix = Mylib::Matrix<TransformComponent*>(rows, cols, nullptr);

	for (pugi::xml_node tileset_node = map_node.child("tileset"); tileset_node; tileset_node = tileset_node.next_sibling("tileset")) {
		const std::string_view tsx_fname = tileset_node.attribute("source").as_string();
		const uint32_t first_gid = tileset_node.attribute("firstgid").as_uint();

		std::filesystem::path folder = std::filesystem::path(tmx_fname).parent_path();
		std::filesystem::path tsx_path = folder / tsx_fname;
		const std::string tsx_fname_str = tsx_path.string();

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(tsx_fname_str.c_str());

		mylib_assert_exception_msg_args(result, FileException, "Error loading TSX file.", tsx_fname)

		pugi::xml_node root_node = doc.child("tileset");
		mylib_assert_exception_msg_args(root_node, FileException, "Tileset node does not exist in TSX file.", tsx_fname)

		const uint32_t tileset_count = root_node.attribute("tilecount").as_uint();
		const uint32_t tileset_cols = root_node.attribute("columns").as_uint();
		const uint32_t tileset_rows = tileset_count / tileset_cols;

		pugi::xml_node image_node = root_node.child("image");
		mylib_assert_exception_msg_args(image_node, FileException, "Image node does not exist in TSX file.", tsx_fname)

		const std::string_view image_fname = image_node.attribute("source").as_string();
		const std::filesystem::path image_path = folder / image_fname;
		const std::string image_fname_str = image_path.string();

		TextureDescriptor texture = renderer->find_texture_by_fname(image_fname_str);
		this->textures.push_back(texture);
		auto matrix = renderer->split_texture(texture, tileset_rows, tileset_cols);

		uint32_t gid = first_gid;
		for (uint32_t i = 0; i < tileset_rows; i++) {
			for (uint32_t j = 0; j < tileset_cols; j++) {
				tile_textures[gid] = matrix[i, j];
				gid++;
			}
		}

		dprintln("Tileset: ", tsx_fname, " firstgid: ", first_gid, " image: ", image_fname_str,
			" rows: ", tileset_rows, " cols: ", tileset_cols);
	}

	bool found_layer = false;

	for (pugi::xml_node layer_node = map_node.child("layer"); layer_node; layer_node = layer_node.next_sibling("layer")) {
		const std::string_view layer_name_attr = layer_node.attribute("name").as_string();

		if (layer_name != layer_name_attr)
			continue;
		
		found_layer = true;
		pugi::xml_node data_node = layer_node.child("data");
		mylib_assert_exception_msg_args(data_node, FileException, "Data node does not exist in TMX file.", tmx_fname)

		const std::string_view encoding = data_node.attribute("encoding").as_string();
		mylib_assert_exception_msg_args(encoding == "csv", FileException, "Unsupported encoding in TMX file.", tmx_fname)

		const std::string data = data_node.text().as_string();
		dprintln("Layer: ", layer_name, " data:\n", data);

		// Parse the CSV data
		std::istringstream csv_stream(data);
		std::string line;
		uint32_t row = 0;

		while (std::getline(csv_stream, line) && row < rows) {
			std::istringstream line_stream(line);
			std::string cell;
			uint32_t col = 0;

			while (std::getline(line_stream, cell, ',') && col < cols) {
				// Remove whitespace from cell
				cell.erase(std::remove_if(cell.begin(), cell.end(), ::isspace), cell.end());
				
				if (!cell.empty()) {
					uint32_t gid = std::stoul(cell);
					
					// GID 0 means empty tile, skip it
					if (gid != 0) {
						auto texture_it = tile_textures.find(gid);
						if (texture_it != tile_textures.end()) {
							this->set(row, col, texture_it->second);
						} else {
							dprintln("Warning: GID ", gid, " not found in tilesets at row ", row, " col ", col);
						}
					}
				}
				col++;
			}
			row++;
		}
	}

	mylib_assert_exception_msg_args(found_layer, FileException, "Layer not found in TMX file.", tmx_fname);
}

// ---------------------------------------------------

void TileMap::set (const uint32_t row, const uint32_t col, TextureDescriptor texture)
{
	auto sprite = make_unique<Sprite2DRenderer>(this->tile_size, texture);
	this->set(row, col, std::move(sprite));
}

// ---------------------------------------------------

} // namespace Game
} // namespace MyGlib