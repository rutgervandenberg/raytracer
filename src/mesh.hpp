#pragma once
#include <xmmintrin.h>

struct Mesh {
	// stored in sets of three
	__m128* polys;
	
	// temporary, for testing
	__m128* colors;

	Mesh(const char* path);
	~Mesh();
};