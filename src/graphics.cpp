#include <my-game-lib/graphics.h>
#include <my-game-lib/debug.h>

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

static CircleFactoryManager circle_factory_manager(10, 50, 1000);

// ---------------------------------------------------

void Cube3D::calculate_vertices () noexcept
{
	std::array<Point, 8> points;
	const fp_t half_w = this->get_w() * fp(0.5);
	const fp_t half_h = this->get_h() * fp(0.5);
	const fp_t half_d = this->get_d() * fp(0.5);

	// front side

	points[LeftTopFront] = Point(
		-half_w,
		half_h,
		-half_d
		);
	
	points[LeftBottomFront] = Point(
		-half_w,
		-half_h,
		-half_d
		);
	
	points[RightTopFront] = Point(
		half_w,
		half_h,
		-half_d
		);
	
	points[RightBottomFront] = Point(
		half_w,
		-half_h,
		-half_d
		);

	// back side
	
	points[LeftTopBack] = Point(
		-half_w,
		half_h,
		half_d
		);
	
	points[LeftBottomBack] = Point(
		-half_w,
		-half_h,
		half_d
		);
	
	points[RightTopBack] = Point(
		half_w,
		half_h,
		half_d
		);
	
	points[RightBottomBack] = Point(
		half_w,
		-half_h,
		half_d
		);
	
	if (this->get_rotation_angle() != fp(0)) {
		for (auto& p : points)
			p.rotate_around_axis(this->get_ref_rotation_axis(), this->get_rotation_angle());
	}

	uint32_t i = 0;

	auto mount = [&i, this, &points] (const PositionIndex p) -> void {
		this->vertices[i] = points[p];
		i++;
	};

	auto mount_triangle = [&mount] (const PositionIndex p1, const PositionIndex p2, const PositionIndex p3) -> void {
		mount(p1);
		mount(p2);
		mount(p3);
	};

	// bottom
	mount_triangle(LeftBottomFront, RightBottomFront, LeftBottomBack);
	mount_triangle(RightBottomBack, RightBottomFront, LeftBottomBack);

	// top
	mount_triangle(LeftTopFront, RightTopFront, LeftTopBack);
	mount_triangle(RightTopBack, RightTopFront, LeftTopBack);

	// front
	mount_triangle(LeftTopFront, LeftBottomFront, RightTopFront);
	mount_triangle(RightBottomFront, LeftBottomFront, RightTopFront);

	// back
	mount_triangle(LeftTopBack, LeftBottomBack, RightTopBack);
	mount_triangle(RightBottomBack, LeftBottomBack, RightTopBack);

	// left
	mount_triangle(LeftTopFront, LeftBottomFront, LeftTopBack);
	mount_triangle(LeftBottomBack, LeftBottomFront, LeftTopBack);

	// right
	mount_triangle(RightTopFront, RightBottomFront, RightTopBack);
	mount_triangle(RightBottomBack, RightBottomFront, RightTopBack);

	//mylib_assert_exception(i == get_n_vertices)
}

// ---------------------------------------------------

void Circle2D::setup_vertices_buffer (const uint32_t n_vertices)
{
	if (this->vertices.size() != n_vertices) {
		this->vertices.resize(n_vertices);
		this->set_vertices(this->vertices);
	}
}

void Circle2D::calculate_vertices (const CircleFactory& factory)
{
	const uint32_t n_vertices = factory.get_n_vertices();
	this->setup_vertices_buffer(n_vertices);
	//dprintln("circle_size_per_cent_of_screen: ", circle_size_per_cent_of_screen, " n_triangles: ", n_vertices / 3);
	factory.build_circle(radius, std::span<Point>(this->vertices));
}

void Circle2D::calculate_vertices (const Matrix4& projection_matrix)
{
	const fp_t radius = this->get_radius();
	const Vector4 circle_left (-radius, 0, 0, 1);
	const Vector4 circle_right (radius, 0, 0, 1);
	const Vector4 circle_left_rendered = projection_matrix * circle_left;
	const Vector4 circle_right_rendered = projection_matrix * circle_right;
	const fp_t circle_size_per_cent_of_screen = std::abs(circle_right_rendered.x - circle_left_rendered.x) / fp(2); // opengl viewport size is 2

	const CircleFactory& factory = circle_factory_manager.get_factory(circle_size_per_cent_of_screen);
	this->calculate_vertices(factory);
}

void Circle2D::calculate_vertices ()
{
	const CircleFactory& factory = circle_factory_manager.get_factory(0.5);
	this->calculate_vertices(factory);
}

// ---------------------------------------------------

void Rect2D::calculate_vertices () noexcept
{
	const fp_t half_w = this->get_w() * fp(0.5);
	const fp_t half_h = this->get_h() * fp(0.5);

	// upper left vertex
	this->vertices[0].x = -half_w;
	this->vertices[0].y = -half_h;
	this->vertices[0].z = this->z;

	// down right vertex
	this->vertices[1].x = half_w;
	this->vertices[1].y = half_h;
	this->vertices[1].z = this->z;

	// down left vertex
	this->vertices[2].x = -half_w;
	this->vertices[2].y = half_h;
	this->vertices[2].z = this->z;

	// draw second triangle

	// upper left vertex
	this->vertices[3].x = -half_w;
	this->vertices[3].y = -half_h;
	this->vertices[3].z = this->z;

	// upper right vertex
	this->vertices[4].x = half_w;
	this->vertices[4].y = -half_h;
	this->vertices[4].z = this->z;

	// down right vertex
	this->vertices[5].x = half_w;
	this->vertices[5].y = half_h;
	this->vertices[5].z = this->z;
}

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
{
	this->factories.reserve(n_cats);

	const fp_t delta = static_cast<fp_t>(max_n_triangles - min_n_triangles) / static_cast<fp_t>(n_cats);

	for (uint32_t i=0; i<n_cats; i++) {
		const uint32_t n_triangles = min_n_triangles + (i * static_cast<uint32_t>(delta));
		this->factories.emplace_back(n_triangles);
	}
}

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib