#include "octree.hpp"
#include <cstdio>
#include <xmmintrin.h>
#include <immintrin.h>
#include "raytrace.h"

Octree::Octree(__m128& position, float halfsize, unsigned int max_depth) : sub(0), position(position), halfsize(halfsize), max_depth(max_depth) {
}

Octree::~Octree() {
	//TODO: recursive delete
	delete[] sub;
}

int Octree::follow(const __m128& v) {
	int index = 0;
	int val[4];
	__m128 r1 = _mm_add_ps(v, _mm_set_ps1(halfsize));
	__m128 r2 = _mm_cmplt_ps(v, r1);
	_mm_store_ps((float*) val, r2);
	for (int a = 0; a < 3; a++) {
			index |= 1 << val[a];
	}

	return index;
}

void Octree::split() {
	// if already subdivided, return
	if (sub)
		return;

	// make leaves
	sub = new Octree*[8];
	for (int x = 0; x < 2; x++){
		for (int y = 0; y < 2; y++){
			for (int z = 0; z < 2; z++){
				__m128 subpos(position + _mm_setr_ps(x * halfsize, y * halfsize, z * halfsize, 1));
				float subhalfsize = halfsize * 0.5f;

				sub[z * 4 + y * 2 + x] = new Octree(subpos, subhalfsize, halfsize);
			}
		}
	}
}

bool Octree::collidePlane(int axis, const __m128& orig, const __m128& dir) {
	float positionarr[4];
	_mm_store_ps(positionarr, position);
	float dirarr[4];
	_mm_store_ps(dirarr, dir);
	float origarr[4];
	_mm_store_ps(origarr, orig);


	// check axis plane
	float v1 = positionarr[axis];

	// flip
	bool flip = (dirarr[axis] < 0);
	//if (dir.p[axis] > 0 && orig.p[axis] > v1) flip = !flip;
	//if (dir.p[axis] < 0 && orig.p[axis] < v1 + 2 * halfsize) flip = !flip;
	v1 += flip * 2 * halfsize;

	// skip
	if ((v1 - orig[axis] - flip * 2 * halfsize) / dirarr[axis] < 0)
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
	if (positionarr[axis2] < v2 && v2 < positionarr[axis2] + 2 * halfsize)
	if (positionarr[axis3] < v3 && v3 < positionarr[axis3] + 2 * halfsize)
		return true;
	return false;
}

bool Octree::collide(const __m128& orig, const __m128& dir) {
	for (int i = 0; i < 3; i++)
		if (collidePlane(i, orig, dir))
			return true;
	return false;
}

float Octree::hit(const __m128& orig, const __m128& dir, triangle* out) {
	// check hit with this cube
	if (!collide(orig, dir)) {
		*out = 0;
		return 1e10f;
	}

	// current closest triangle
	float shortest = 1e10f;
	triangle res = 0;

	// check with leaves
	for (unsigned int i = 0; i < leaves.size(); i++) {
		const float dist = intersect(orig, dir, leaves[i]);
		if (dist && dist < shortest) {
			shortest = dist;
			res = leaves[i];
		}
	}

	// check with subnodes
	// TODO: skip irrelevant subnodes
	if (sub) {
		for (int i = 0; i < 8; i++) {
			triangle res2;
			float dist = sub[i]->hit(orig, dir, &res2);
			if (dist < shortest) {
				res = res2;
				shortest = dist;
			}
		}
	}

	// hit
	*out = res;
	return shortest;
}

void Octree::calcSize(Mesh& mesh) {
	__m128 p1; // min
	__m128 p2; // max

	for (unsigned int i = 0; i < mesh.numtriangles; i++) { // triangle
		for (int v = 0; v < 3; v++) { // vertex
			for (int d = 0; d < 3; d++) { // dimension
				__m128 vertex = mesh.triangles[i*3+v]; //mesh.triangles[i].vertices[v].position.p[d];
				float temp[4];
				_mm_store_ps(temp, vertex);
				//if (temp[d] < p1.p[d])
				//	p1.p[d] = dim;
				//if (dim > p2.p[d])
				//	p2.p[d] = dim;
			}
		}
	}
//	printf("min: (%.1f,%.1f,%.1f)\n", p1.p[X], p1.p[Y], p1.p[Z]);
//	printf("max: (%.1f,%.1f,%.1f)\n", p2.p[X], p2.p[Y], p2.p[Z]);

	__m128 dim = _mm_sub_ps(p2, p1);
	float temp[4];
	_mm_store_ps(temp, dim);
	float sdim = temp[0];
	sdim = fmax(sdim, temp[1]);
	sdim = fmax(sdim, temp[2]);
}

void Octree::build(Mesh& mesh) {
	max_depth = (int)(log(mesh.numtriangles)) / log(2);
	printf("building tree with size %d\n", max_depth);
	calcSize(mesh);

	for (unsigned int i = 0; i < mesh.numtriangles; i++)
		add(&mesh.triangles[i]);
}

void Octree::add(triangle tr) {
	Octree* current = this;
	int a0, a1, a2;

	unsigned int depth = 0;
	while (depth < max_depth) {
		a0 = current->follow(tr[0]);
		a1 = current->follow(tr[1]);
		a2 = current->follow(tr[2]);

		if (a0 != a1 || a1 != a2)
			break;

		current->split();
		current = current->sub[a0];
		depth++;
	}

	current->leaves.push_back(tr);
}
