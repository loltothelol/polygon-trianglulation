#pragma once

template <typename T>
struct triangle
{
	T A, B, C;
};

namespace geom
{
	struct vec2
	{
		double x, y;
	};

	static inline double sign(const vec2 & p1, const vec2 & p2, const vec2 & p3)
	{
	    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	}

	static bool point_in_triangle(const triangle<vec2> & tri, const vec2 & pt)
	{
	    double d1 = sign(pt, tri.A, tri.B);
	    double d2 = sign(pt, tri.B, tri.C);
	    double d3 = sign(pt, tri.C, tri.A);

	    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	    return !(has_neg && has_pos);
	}

	static inline double det(const vec2 A, const vec2 B, const vec2 C)
	{
		const vec2 AB = { B.x - A.x, B.y - A.y };
		const vec2 BC = { C.x - B.x, C.y - B.y };
		return (AB.x * BC.y) - (BC.x * AB.y);
	}

	static inline double det(const triangle<vec2> & tri)
	{
		return det(tri.A, tri.B, tri.C);
	}

	static inline bool is_convex(const triangle<vec2> & tri)
	{
		return det(tri) > 0;
	}

	static inline bool is_reflex(const triangle<vec2> & tri)
	{
		return det(tri) < 0;
	}
}