#include "octree.hpp"
#include <cstdio>
#include <xmmintrin.h>
#include <immintrin.h>
#include "raytrace.hpp"

Octree::Octree(vec4& pos, float halfsize, int maxdepth) :
		sub(0), pos(pos), halfsize(halfsize), maxdepth(maxdepth) {
	// nothing to do
}

Octree::Octree(const Mesh& mesh) {
	maxdepth = (int)(log(mesh.numtriangles)) / log(2);
	calcSize(mesh);

	//for (int i = 0; i < mesh.numtriangles; i++)
	//	add(&mesh.triangles[i * 3]);
}

// destroy subnodes
Octree::~Octree() {
	if (sub) {
		for (int i = 0; i < 8; i++)
			delete sub[i];
		delete[] sub;
	}
}

// compare each axis
int Octree::follow(const vec4& v) {
	int index = 0;
	int val[4];
	vec4 r1 = pos + vec4s(halfsize);
	vec4 r2 = v < r1;
	_mm_store_ps((float*) val, r2);

	for (int a = 0; a < 3; a++)
		if (val[a])
			index |= 1 << a;

	return index;
}

// early return
void Octree::split() {
	// if already subdivided, return
	if (sub)
		return;

	// make leaves
	sub = new Octree*[8];
	for (int x = 0; x < 2; x++){
		for (int y = 0; y < 2; y++){
			for (int z = 0; z < 2; z++){
				vec4 subpos = pos + vec4(x * halfsize, y * halfsize, z * halfsize, 1);
				float subhalfsize = halfsize * 0.5f;
				sub[z * 4 + y * 2 + x] = new Octree(subpos, subhalfsize, maxdepth - 1);
			}
		}
	}
}

void Octree::print() {
	print(0);
}

void Octree::print(int tabs) {
	// current depth
	for (int i = 0; i < tabs; i++) printf("  ");

	// print self
	printf("%d\n", leaves.size());

	// print children
	if (sub)
	for (int i = 0; i < 8; i++)
		sub[i]->print(tabs + 1);
}

bool Octree::collidePlane(int axis, const vec4& orig, const vec4& dir) const {
	__declspec(align(16)) float posarr[4];
	_mm_store_ps(posarr, pos);
	__declspec(align(16)) float dirarr[4];
	_mm_store_ps(dirarr, dir);
	__declspec(align(16)) float origarr[4];
	_mm_store_ps(origarr, orig);


	// check axis plane
	float v1 = posarr[axis];

	// flip
	bool flip = (dirarr[axis] < 0);
	//if (dir.p[axis] > 0 && orig.p[axis] > v1) flip = !flip;
	//if (dir.p[axis] < 0 && orig.p[axis] < v1 + 2 * halfsize) flip = !flip;
	v1 += flip * 2 * halfsize;

	// skip
	if ((v1 - origarr[axis] - flip * 2 * halfsize) / dirarr[axis] < 0)
		return false;

	// factor
	float a = (v1 - origarr[axis]) / dirarr[axis];

	// behind the plane
	//if (a < 0)
	//	return false;

	// other axis
	int axis2 = (axis + 1) % 3;
	int axis3 = (axis + 2) % 3;

	// other axis values
	float v2 = a * dirarr[axis2] + origarr[axis2];
	float v3 = a * dirarr[axis3] + origarr[axis3];

	// check 2D Octree
	if (posarr[axis2] < v2 && v2 < posarr[axis2] + 2 * halfsize)
	if (posarr[axis3] < v3 && v3 < posarr[axis3] + 2 * halfsize)
		return true;
	return false;
}

bool Octree::collide(const vec4& orig, const vec4& dir) const {
	for (int i = 0; i < 3; i++)
		if (collidePlane(i, orig, dir))
			return true;
	return false;
}

vec4 Octree::hit(const vec4& orig, const vec4& dir, triangle* out) const {

	__declspec(align(16)) int temp;

	// check hit with this cube
	if (!collide(orig, dir)) {
		*out = 0;
		return vec4s(1e10f);
	}

	// current closest triangle
	vec4 shortest = vec4s(1e10f);
	triangle res = 0;

	// check with leaves
	for (unsigned i = 0; i < leaves.size(); i++) {
		vec4 dist = intersect(orig, dir, leaves[i]);
		_mm_store_ss((float*)&temp, dist & dist < shortest);
		if (temp) {
			shortest = dist;
			res = leaves[i];
		}
	}

	// check with subnodes
	// TODO: skip irrelevant subnodes
	if (sub) {
		for (int i = 0; i < 8; i++) {
			triangle res2;
			vec4 dist = sub[i]->hit(orig, dir, &res2);
			vec4 eq = dist & dist < shortest;
			temp = 0;
			_mm_store_ss((float*)&temp, eq);
			if (temp) {
				shortest = dist;
				res = res2;
			}
		}
	}

	// hit
	*out = res;
	return shortest;
}

void Octree::calcSize(const Mesh& mesh) {
	vec4 p1 = vec4s(1e10f); // min
	vec4 p2 = vec4s(-1e10f); // max

	for (int i = 0; i < mesh.numtriangles; i++) { // triangle
		vec4 v[3];
		v[0] = mesh.triangles[i * 3 + 0];
		v[1] = mesh.triangles[i * 3 + 1] - v[0];
		v[2] = mesh.triangles[i * 3 + 2] - v[0];

		for (int i = 0; i < 3; i++) {
			p1 = min(p1, v[i]);
			p2 = max(p2, v[i]);
		}
	}
//	printf("min: (%.1f,%.1f,%.1f)\n", p1.p[X], p1.p[Y], p1.p[Z]);
//	printf("max: (%.1f,%.1f,%.1f)\n", p2.p[X], p2.p[Y], p2.p[Z]);

	vec4 dim = p2 - p1;

	float temp[4];
	*(vec4*)temp = dim;
	halfsize = fmax(temp[0], temp[1]);
	halfsize = fmax(halfsize, temp[2]);

	// result
	halfsize *= 0.5f;
	pos = p1;
}

void Octree::add(triangle tr) {
	Octree* current = this;
	int a0, a1, a2;

	int depth = 0;
	while (depth < maxdepth) {
		a0 = current->follow(tr[0]);
		a1 = current->follow(tr[1] + tr[0]);
		a2 = current->follow(tr[2] + tr[0]);

		if (a0 != a1 || a1 != a2)
			break;

		current->split();
		current = current->sub[a0];
		depth++;
	}

	current->leaves.push_back(tr);
}
