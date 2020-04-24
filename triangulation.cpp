#include <iterator>
#include <set>
#include <vector>

#include "geom.hpp"

// ths function returns a triangle built from the two neighbours of the element at index 'i'
template <typename T>
static inline triangle<T> build_triangle_at_index(const std::vector<T> & v, const std::size_t i)
{
	if (i >= v.size())
		throw std::out_of_range("build_triangle_at_index: index is out-of-range.");

	triangle<T> triangle;

	triangle.A = v[(i > 0) ? (i - 1) : (v.size() - 1)];
	triangle.B = v[i];
	triangle.C = v[(i < v.size() - 1) ? (i + 1) : (v.size() - 1)];

	return triangle;
}

// this function looks for an element in the vector 'v', and returns a triangle ABC, where B
// is the original element 'e', and A and C are 'e's neighbours
template <typename T>
static inline triangle<T> build_triangle_at_element(const std::vector<T> & v, const T & e)
{
	auto it = std::find(v.begin(), v.end(), e);
	if (it == v.end())
		throw std::logic_error("build_triangle_at_element: element could not be found.");

	const std::size_t i = std::distance(v.begin(), it);

	return build_triangle_at_index(v, i);
}

// this function takes a fake triangle (made up of indices or pointers to a vertex array) and 
// returns a new triangle that is filled with the real vertices (i.e. vectors) at those indices
template <typename T>
static inline triangle<T> make_triangle_real(const std::vector<T> & v, const triangle<std::size_t> tri)
{
	triangle<T> result;

	result.A = v.at(tri.A);
	result.B = v.at(tri.B);
	result.C = v.at(tri.C);

	return result;
}

// a fake triangle is one whose vertices serve as indices (pointers) to the real vertices
using fake_triangle = triangle<std::size_t>;

// a real triangle, in contrast, is one whose vertices are real (i.e. expressed as vectors)
using real_triangle = triangle<geom::vec2>;

class polygon
{
public:
	polygon(const std::vector<geom::vec2> & vertices) :
		vertices(vertices)
	{
		for (std::size_t i = 0; i < vertices.size(); ++i)
			vertex_list.push_back(i);

		for (std::size_t vertex : vertex_list)
		{
			real_triangle ABC = build_real_triangle_at(vertex);

			if (geom::is_convex(ABC))
				convex_list.insert(vertex);

			if (geom::is_reflex(ABC))
				reflex_list.insert(vertex);
		}

		for (std::size_t vertex : convex_list)
			if (is_ear(vertex, false))
				ear_list.insert(vertex);
	}

	fake_triangle build_fake_triangle_at(const std::size_t vertex) const
	{
		return build_triangle_at_element(vertex_list, vertex);
	}

	real_triangle make_triangle_real(const fake_triangle & fake) const
	{
		return ::make_triangle_real(vertices, fake);
	}

	real_triangle build_real_triangle_at(const std::size_t vertex) const
	{
		return make_triangle_real(build_fake_triangle_at(vertex));
	}

	bool is_convex(const std::size_t vertex) const
	{
		if (convex_list.find(vertex) != convex_list.end())
			return true;

		// manually test for convexity
		return geom::is_convex(build_real_triangle_at(vertex));
	}

	bool is_reflex(const std::size_t vertex) const
	{
		if (reflex_list.find(vertex) != reflex_list.end())
			return true;

		// manually test for concavity
		return geom::is_reflex(build_real_triangle_at(vertex));
	}

	bool is_ear(const std::size_t vertex, bool pre_test = true) const
	{
		if (pre_test && ear_list.find(vertex) != ear_list.end())
			return true;

		real_triangle ABC = build_real_triangle_at(vertex);

		// test if any reflex vertices are contained in the triangle ABC
		for (std::size_t reflex_vertex : reflex_list)
			if (point_in_triangle(ABC, vertices[reflex_vertex]))
				return false;

		return true;
	}

	void update_vertex(const std::size_t vertex)
	{
		// if the vertex is convex
		if (update_vertex_convexity(vertex))
		{
			// test if it is an ear
			update_vertex_earness(vertex);
		}
	}

	fake_triangle remove_vertex(const std::size_t vertex)
	{
		// build a triangle at the vertex
		const fake_triangle ABC = build_fake_triangle_at(vertex);
		
		// remove the vertex from the vertex list
		vertex_list.erase(std::find(vertex_list.begin(), vertex_list.end(), vertex));
		
		// remove the vertex from the complementary lists
		convex_list.erase(vertex);
		reflex_list.erase(vertex);
		ear_list.erase(vertex);

		for (std::size_t vertex : vertex_list)
		{
			printf("%zu, ", vertex);
		}
		printf("]\n");

		// update neighbouring vertices
		update_vertex(ABC.A);
		update_vertex(ABC.C);

		return ABC;
	}

	bool has_ear() const
	{
		return !ear_list.empty();
	}

	std::size_t get_next_ear() const
	{
		if (ear_list.empty())
			throw std::logic_error("get_next_ear: Ear list is empty.");
		return *(ear_list.begin());
	}

	std::size_t size() const
	{
		return vertex_list.size();
	}

private:
	bool update_vertex_convexity(const std::size_t vertex)
	{
		if (is_convex(vertex)) 
		{
			convex_list.insert(vertex);
			reflex_list.erase(vertex);
			return true;
		} 
		return false;
	}

	bool update_vertex_earness(const std::size_t vertex)
	{
		if (is_ear(vertex, false)) 
		{
			ear_list.insert(vertex);
			return true;
		}
		else
		{
			ear_list.erase(vertex);
			return false;
		}
	}

	const std::vector<geom::vec2> & vertices;
	std::vector<std::size_t> vertex_list;
	std::set<std::size_t> convex_list, reflex_list, ear_list;
};

std::vector<fake_triangle> triangulate(const std::vector<geom::vec2> & vertices)
{
	polygon polygon (vertices);
	std::vector<fake_triangle> triangles;

	while (polygon.size() > 2)
	{
		// if there are no ears left, this violates the two-ears theorem, 
		// and suggests that the polygon is non-simple
		if (!polygon.has_ear())
			throw std::logic_error("Triangulation failed; polygon is non-simple.");

		const std::size_t ear_vertex = polygon.get_next_ear();
		fake_triangle ear = polygon.remove_vertex(ear_vertex);
		triangles.push_back(ear);
	}

	return triangles;
}

int main()
{
	try
	{
		// vertices of test polygon
		std::vector<geom::vec2> vertices =
		{
			{ 2, 0 }, { 1, 0 }, { 0, 1 }, { -1, 0 }
		};

		// triangulate the test polygon
		std::vector<fake_triangle> triangles = triangulate(vertices);

		// print the newly found the triangles
		for (fake_triangle & tri : triangles)
		{
			printf("A: %zu, B: %zu, C: %zu\n", tri.A, tri.B, tri.C);
		}
	}
	catch (std::exception & e)
	{
		printf("%s\n", e.what());
	}
	
}