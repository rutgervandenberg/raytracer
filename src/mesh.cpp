#include "mesh.hpp"
#include "mem.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
using namespace std;

float random1() {
	return (rand() % 255) / 255.0f;
}

Mesh::Mesh(const char* path) {
	// temp data
	vector<__m128> tempvertices(1000000);
	vector<__m128> temptriangles(1000000);
	vector<__m128> tempcolors(1000000);

	// reading
	ifstream in(path);
	string token;

	// go until no token is found
	while (in >> token) {
		// read a vertex
		if (token == "v") {
			float x, y, z;
			in >> x >> y >> z;
			tempvertices.push_back(_mm_set_ps(x, y, z, 1));
		}

		// read a face
		else if (token == "f") {
			unsigned i, j, k;
			in >> i >> j >> k;
			if (i > temptriangles.size() || j > temptriangles.size() || k > temptriangles.size())
				throw "vertex index out of bounds";
			temptriangles.push_back(tempvertices[i]);
			temptriangles.push_back(tempvertices[j]);
			temptriangles.push_back(tempvertices[k]);
			tempcolors.push_back(_mm_set_ps(random1(), random1(), random1(), 1.0f));
		}

		// skip this line
		else {
			getline(in, token);
		}
	}

	// copy to self
	triangles = (__m128*)ialloc(temptriangles.size() * 16);
	memcpy(triangles, &temptriangles[0], temptriangles.size() * 16);

	// colors
	colors = (__m128*)ialloc(temptriangles.size() * 16);
	memcpy(colors, &tempcolors[0], tempcolors.size() * 16);
}

Mesh::~Mesh() {
	ifree(triangles);
	ifree(colors);
}