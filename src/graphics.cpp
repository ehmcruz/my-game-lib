#include <ostream>
#include <array>
#include <utility>

#include <my-game-lib/graphics.h>
#include <my-game-lib/debug.h>

namespace MyGlib
{
namespace Graphics
{

// ---------------------------------------------------

static CircleFactoryManager circle_factory_manager(10, 20, 80);

// ---------------------------------------------------

const char* Manager::get_type_str (const Type value)
{
	static constexpr auto strs = std::to_array<const char*>({
	#ifdef MYGLIB_SUPPORT_SDL
		"SDL",
	#endif
	#ifdef MYGLIB_SUPPORT_OPENGL
		"Opengl",
	#endif
	#ifdef MYGLIB_SUPPORT_VULKAN
		"Vulkan",
	#endif
		"Unsupported" // must be the last one
	});

	mylib_assert_exception_msg(std::to_underlying(value) < strs.size(), "invalid enum class value ", std::to_underlying(value))

	return strs[ std::to_underlying(value) ];

}

// ---------------------------------------------------

std::ostream& operator << (std::ostream& out, const Color& color)
{
	out << "[" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << "]";
	return out;
}

// ---------------------------------------------------

void Shape::calculate_rotation ()
{
	this->must_recalculate_rotation = false;

	mylib_assert_exception_msg(this->type != Type::Sphere3D,
		"We rotate Spheres3D in a shader");

	Matrix3 rotation_matrix = Mylib::Math::gen_rotation_matrix(this->rotation_axis, this->rotation_angle);

	for (uint32_t i = 0; i < this->local_vertices_buffer__.size(); i++) {
		this->local_rotated_vertices_buffer__[i].pos = rotation_matrix * this->local_vertices_buffer__[i].pos;
		this->local_rotated_vertices_buffer__[i].normal = rotation_matrix * this->local_vertices_buffer__[i].normal;
	}
}

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

	uint32_t i = 0;

	auto mount = [&i, this, &points] (const PositionIndex p, const Vector& normal) -> void {
		this->vertices[i].pos = points[p];
		this->vertices[i].normal = normal;
		i++;
	};

	auto mount_triangle = [&mount] (const PositionIndex p1, const PositionIndex p2, const PositionIndex p3, const Vector& normal) -> void {
		mount(p1, normal);
		mount(p2, normal);
		mount(p3, normal);
	};

	// p1 and p2 should be a diagonal of the rectangle
	auto mount_surface = [&mount_triangle] (const PositionIndex p1, const PositionIndex p2, const PositionIndex p3, const PositionIndex p4, const Vector& normal) -> void {
		mount_triangle(p1, p2, p3, normal);
		mount_triangle(p1, p2, p4, normal);
	};

	// bottom
	mount_surface(LeftBottomFront, RightBottomBack, RightBottomFront, LeftBottomBack, Vector(0, -1, 0));

	// top
	mount_surface(LeftTopFront, RightTopBack, RightTopFront, LeftTopBack, Vector(0, 1, 0));

	// front
	mount_surface(LeftTopFront, RightBottomFront, RightTopFront, LeftBottomFront, Vector(0, 0, -1));

	// back
	mount_surface(LeftTopBack, RightBottomBack, RightTopBack, LeftBottomBack, Vector(0, 0, 1));

	// left
	mount_surface(LeftTopFront, LeftBottomBack, LeftTopBack, LeftBottomFront, Vector(-1, 0, 0));

	// right
	mount_surface(RightTopFront, RightBottomBack, RightTopBack, RightBottomFront, Vector(1, 0, 0));

	this->force_recalculate_rotation();
}

// ---------------------------------------------------

void Sphere3D::setup_vertices_buffer (const uint32_t n_vertices)
{
	if (this->vertices.size() != n_vertices) {
		this->vertices.resize(n_vertices);
		this->set_vertices_buffer(this->vertices, this->vertices);
	}
}

void Sphere3D::calculate_vertices ()
{
	/*
		The code here is very inneficient.
		Let's leave as it is right now.
		Later we are goind to write a shader specific for spheres
		and speed things up.
	*/

	constexpr uint32_t v_resolution = 50; // latitude
	constexpr uint32_t u_resolution = v_resolution * 2; // longitude

	constexpr fp_t start_u = 0;
	constexpr fp_t start_v = 0;

	constexpr fp_t end_u = std::numbers::pi_v<fp_t> * fp(2);
	constexpr fp_t end_v = std::numbers::pi_v<fp_t>;

	constexpr fp_t step_u = (end_u - start_u) / static_cast<fp_t>(u_resolution);
	constexpr fp_t step_v = (end_v - start_v) / static_cast<fp_t>(v_resolution);

	const fp_t radius = this->get_radius();

	auto f = [] (const fp_t u, const fp_t v, const fp_t r) -> Point {
		return Point(
			std::cos(u) * std::sin(v) * r,
			std::cos(v) * r,
			std::sin(u) * std::sin(v) * r
		);
	};

	this->setup_vertices_buffer(u_resolution * v_resolution * 6);

	uint32_t k = 0;

	for (uint32_t i = 0; i < u_resolution; i++) {
		for (uint32_t j = 0; j < v_resolution; j++) {
			const fp_t u = static_cast<fp_t>(i) * step_u + start_u;
			const fp_t v = static_cast<fp_t>(j) * step_v + start_v;
			const fp_t un = (i + 1 == u_resolution) ? end_u : static_cast<fp_t>(i + 1) * step_u + start_u;
			const fp_t vn = (j + 1 == v_resolution) ? end_v : static_cast<fp_t>(j + 1) * step_v + start_v;

			const Point p0 = f(u, v, radius);
			const Point p1 = f(u, vn, radius);
			const Point p2 = f(un, v, radius);
			const Point p3 = f(un, vn, radius);

			// For spheres, the normal is just the normalized
			// version of each vertex point.

			// Output the first triangle of this grid square

			this->vertices[k].pos = p0;
			this->vertices[k].normal = normalize(p0);

			this->vertices[k + 1].pos = p2;
			this->vertices[k + 1].normal = normalize(p2);

			this->vertices[k + 2].pos = p1;
			this->vertices[k + 2].normal = normalize(p1);

			// Output the other triangle of this grid square

			this->vertices[k + 3].pos = p3;
			this->vertices[k + 3].normal = normalize(p3);

			this->vertices[k + 4].pos = p1;
			this->vertices[k + 4].normal = normalize(p1);

			this->vertices[k + 5].pos = p2;
			this->vertices[k + 5].normal = normalize(p2);

			k += 6;
		}
	}

	// no need to force recalculate rotation
	// we will rotate the sphere in a shader
}

// ---------------------------------------------------

void Circle2D::setup_vertices_buffer (const uint32_t n_vertices)
{
	if (this->vertices.size() != n_vertices) {
		this->vertices.resize(n_vertices);
		this->rotated_vertices.resize(n_vertices);
		this->set_vertices_buffer(this->vertices, this->rotated_vertices);
	}
}

void Circle2D::calculate_vertices (const CircleFactory& factory)
{
	const uint32_t n_vertices = factory.get_n_vertices();
	this->setup_vertices_buffer(n_vertices);
	//dprintln("circle_size_per_cent_of_screen: ", circle_size_per_cent_of_screen, " n_triangles: ", n_vertices / 3);
	factory.build_circle(radius, std::span<Vertex>(this->vertices));

	for (auto& v : this->vertices) {
		v.normal = Vector(0, 0, -1);
		v.pos.z = 0;
	}

	this->force_recalculate_rotation();
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
	constexpr fp_t z = 0;

	// upper left vertex
	this->vertices[0].pos.x = -half_w;
	this->vertices[0].pos.y = -half_h;
	this->vertices[0].pos.z = z;

	// down right vertex
	this->vertices[1].pos.x = half_w;
	this->vertices[1].pos.y = half_h;
	this->vertices[1].pos.z = z;

	// down left vertex
	this->vertices[2].pos.x = -half_w;
	this->vertices[2].pos.y = half_h;
	this->vertices[2].pos.z = z;

	// draw second triangle

	// upper left vertex
	this->vertices[3].pos.x = -half_w;
	this->vertices[3].pos.y = -half_h;
	this->vertices[3].pos.z = z;

	// upper right vertex
	this->vertices[4].pos.x = half_w;
	this->vertices[4].pos.y = -half_h;
	this->vertices[4].pos.z = z;

	// down right vertex
	this->vertices[5].pos.x = half_w;
	this->vertices[5].pos.y = half_h;
	this->vertices[5].pos.z = z;

	for (auto& v : this->vertices)
		v.normal = Vector(0, 0, -1);

	this->force_recalculate_rotation();
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

void CircleFactory::build_circle (const fp_t radius, std::span<Vertex> vertices) const
{
	uint32_t j;
	fp_t previous_x, previous_y;

	/*
		For each triangle:
			- first vertex is the center (0.0f, 0.0f)
			- second vertex is the previous calculated vertex (from previous triangle)
			- third vertex is the new vertex
	*/

	// for the first triangle
	previous_x = radius;
	previous_y = 0;

	j = 0;
	for (uint32_t i=0; i<this->n_triangles; i++) {
		// first vertex
		vertices[j].pos.x = 0;
		vertices[j].pos.y = 0;

		j++;

		// second vertex
		vertices[j].pos.x = previous_x;
		vertices[j].pos.y = previous_y;

		j++;

		// third vertex
		vertices[j].pos.x = this->table_cos[i] * radius;
		vertices[j].pos.y = this->table_sin[i] * radius;

		previous_x = vertices[j].pos.x;
		previous_y = vertices[j].pos.y;

		j++;
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

LightPointDescriptor Manager::add_light_point_source (const Point& pos, const Color& color)
{
	for (uint32_t id = 0; auto& light_source : this->light_point_sources) {
		if (light_source.busy == false) {
			light_source.busy = true;
			light_source.pos = pos;
			light_source.color = color;
			return id;
		}
		id++;
	}

	mylib_throw_exception_msg("no more light point sources available");
}

// ---------------------------------------------------

} // end namespace Graphics
} // end namespace MyGlib