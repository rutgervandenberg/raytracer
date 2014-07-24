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
	vector<__m128> tempverts(1000000);
	vector<__m128> temppolys(1000000);
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
			temppolys.push_back(_mm_set_ps(x, y, z, 1));
		}

		// read a face
		else if (token == "f") {
			unsigned i, j, k;
			in >> i >> j >> k;
			if (i > temppolys.size() || j > temppolys.size() || k > temppolys.size())
				throw "non sequitur";
			temppolys.push_back(tempverts[i]);
			temppolys.push_back(tempverts[j]);
			temppolys.push_back(tempverts[k]);
			tempcolors.push_back(_mm_set_ps(random1(), random1(), random1(), 1.0f));
		}

		// skip this line
		else {
			getline(in, token);
		}
	}

	// copy to self
	polys = (__m128*)ialloc(temppolys.size() * 16);
	memcpy(polys, &temppolys[0], temppolys.size() * 16);

	// colors
	colors = (__m128*)ialloc(temppolys.size() * 16);
	memcpy(colors, &tempcolors[0], tempcolors.size() * 16);
}

Mesh::~Mesh() {
	ifree(polys);
}