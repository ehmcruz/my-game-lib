#include <my-game-lib/graphics.h>
#include <my-game-lib/debug.h>

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

CircleFactory::CircleFactory (const uint32_t n_triangles_)
	: n_triangles(n_triangles_)
{
	this->table_cos.resize(this->n_triangles);
	this->table_sin.resize(this->n_triangles);

	/*
		cos(angle) = x / radius
		sin(angle) = y / radius
		
		x = cos(angle) * radius
		y = sin(angle) * radius

		2*pi radians is equal to 360 degrees
	*/

	const double delta = (2.0 * std::numbers::pi) / static_cast<double>(this->n_triangles);
	double angle = delta;

/*
	dprint( std::endl )
	dprint( "delta = " << delta << std::endl )
	dprint( std::endl )
*/

	for (uint32_t i=0; i<this->n_triangles; i++) {
		this->table_cos[i] = static_cast<fp_t>( cos(angle) );
		this->table_sin[i] = static_cast<fp_t>( sin(angle) );

	/*
		dprint( "cos(" << angle << ") = " << this->table_cos[i] << std::endl )
		dprint( "sin(" << angle << ") = " << this->table_sin[i] << std::endl )
		dprint( std::endl )
	*/

		angle += delta;
	}
}

// ---------------------------------------------------

CircleFactoryManager::CircleFactoryManager (const uint32_t n_cats, const uint32_t min_n_triangles, const uint32_t max_n_triangles)
: min_n_triangles(min_n_triangles), max_n_triangles(max_n_triangles)
{
	this->factories.reserve(n_cats);

	const fp_t delta = static_cast<fp_t>(max_n_triangles - min_n_triangles) / static_cast<fp_t>(n_cats);

	for (uint32_t i=0; i<n_cats; i++) {
		const uint32_t n_triangles = min_n_triangles + (i * static_cast<uint32_t>(delta));
		this->factories.emplace_back(n_triangles);
	}

	dprintln("CircleFactoryManager created with ", n_cats, " categories");
}

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib