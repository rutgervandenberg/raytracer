#pragma once
#include <xmmintrin.h>
typedef __m128* triangle;

struct Mesh {
	// stored in sets of three
	int numtriangles;
	__m128* triangles;
	
	// temporary, for testing
	int numcolors;
	__m128* colors;

	Mesh(const char* path);
	~Mesh();
};
