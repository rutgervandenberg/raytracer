#pragma once
#include <vector>
#include "mem.hpp"
#include "matrix.hpp"
#include "vector.hpp" /* vector intrinsic math */
using namespace std;

struct TriAccel {
	// normal data
	float nu;
	float nv;
	// vertex data
	float np;
	float pu;
	float pv;
	//edge data
	int ci;
	float e0u;
	float e0v;
	float e1u;
	float e1v;
	// padding
	int pad0;
	int pad1;

	TriAccel(const vec4& v0, const vec4& v1, const vec4& v2) {
		// handy
		vec4 e0 = v1 - v0;
		vec4 e1 = v2 - v0;

		// normal
		vec4 n = cross(e0, e1);
		vec4 len = sqrt(dot(n, n, 0x7F));
		n = n / len;

		// indices
		int w = 0, u = 0, v = 2;
		if (abs(get1(n)) > abs(get0(n))) w = 1;
		if (abs(get2(n)) > abs(get0(n)) && abs(get2(n)) > abs(get1(n))) w = 2;
		if (w == 1) u = 0;
		if (w == 2) v = 1;

		// normal (u,v)
		nu = get(n, u) / get(n, w);
		nv = get(n, v) / get(n, w);
		float nw = get(n, w);

		// vertex (u,v)
		pu = get(v0, u);
		pv = get(v0, v);
		np = nu * pu + nv * pv + get(v0, w);

		// edges (edgy)
		float sign = (w == 1) ? -1.0f : 1.0f;
		e0u = sign * get(e0, u) / nw;
		e0v = sign * get(e0, v) / nw;
		e1u = sign * get(e1, u) / nw;
		e1v = sign * get(e1, v) / nw;

		// bitfield
		ci = w;

		// axis-aligned triangle flag TODO
		//ci |= 0x4;
	}
};

template<typename T, typename I>
struct TriAccelArray {
	// normal data
	T* nu;
	T* nv;
	// vertex data
	T* np;
	T* pu;
	T* pv;
	// edge data
	I* ci;
	T* e0u;
	T* e0v;
	T* e1u;
	T* e1v;
	// padding (template??)
	int pad0;
	int pad1;

	void load(vector<TriAccel>& ref) {
		// duplicate 8 times !!
		int size = ref.size();
		nu = (vec8*)ialloc(size * sizeof T);
		nv = (vec8*)ialloc(size * sizeof T);
		np = (vec8*)ialloc(size * sizeof T);
		pu = (vec8*)ialloc(size * sizeof T);
		pv = (vec8*)ialloc(size * sizeof T);
		ci = (ivec8*)ialloc(size * sizeof T);
		e0u = (vec8*)ialloc(size * sizeof T);
		e0v = (vec8*)ialloc(size * sizeof T);
		e1u = (vec8*)ialloc(size * sizeof T);
		e1v = (vec8*)ialloc(size * sizeof T);

		for (int i = 0; i < size; i++) {
			nu[i] = convert<float,T>(ref[i].nu);
			nv[i] = _mm256_set1_ps(ref[i].nv);
			np[i] = _mm256_set1_ps(ref[i].np);
			pu[i] = _mm256_set1_ps(ref[i].pu);
			pv[i] = _mm256_set1_ps(ref[i].pv);
			ci[i] = _mm256_set1_epi32(ref[i].ci);
			e0u[i] = _mm256_set1_ps(ref[i].e0u);
			e0v[i] = _mm256_set1_ps(ref[i].e0v);
			e1u[i] = _mm256_set1_ps(ref[i].e1u);
			e1v[i] = _mm256_set1_ps(ref[i].e1v);
		}
	}
};

// a type designed for holding a group of rays
template <typename T>
struct raypacket { T t[3]; };

// surface of triangle
// heron formula
inline vec4 surface(vec4* tr) {

	// side lengths
	vec4 a = sqrt(dot(tr[1], tr[1], 0x7F));
	vec4 b = sqrt(dot(tr[2], tr[2], 0x7F));
	vec4 c = sqrt(dot(tr[2] - tr[1], tr[2] - tr[1], 0x7F));

	// om
	vec4 s = vec4s(0.5f) * (a + b + c);

	// area
	vec4 area = sqrt(s * (s - a) * (s - b) * (s - c));
	return area;
}

// surface of triangle
// heron formula
inline vec4 surface(vec4 e1, vec4 e2) {

	// side lengths
	vec4 a = sqrt(dot(e1, e1, 0x7F));
	vec4 b = sqrt(dot(e2, e2, 0x7F));
	vec4 c = sqrt(dot(e2 - e1, e2 - e1, 0x7F));

	// om
	vec4 s = vec4s(0.5f) * (a + b + c);

	// area
	vec4 area = sqrt(s * (s - a) * (s - b) * (s - c));
	return area;
}

/**
* Intersect a packet of 4 rays against a
* triangle
*/
template <typename T, typename I>
T intersectPacket(const TriAccelArray<T, I>& a, int i,
	const raypacket<T>& o, const raypacket<T>& d);

/**
 *	INTERSECT(point, dir, triangle)
 *	Compute the intersection between a
 *	single ray and a triangle.
 *	p = origin of the ray
 *	d = direction of the ray
 *	tr = triangle / array of three vertices
 */
vec4 intersect(vec4 p, vec4 d, vec4* tr);
const static vec4 one = vec4s(1.0f);
const static vec4 zero = vec4s(0.0f);