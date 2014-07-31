#pragma once
#include "vector.hpp"
#include "geometry.hpp"
typedef vec4* triangle;

struct Mesh {
	// stored in sets of three
	int numtriangles;
	vec4* triangles;
	
	// temporary, for testing
	int numcolors;
	vec4* colors;

	Mesh(const char* path);
	~Mesh();
};
