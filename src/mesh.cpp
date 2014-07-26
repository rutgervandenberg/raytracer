#include "mesh.hpp"
#include "mem.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
using namespace std;

float random1() {
	return (rand() % 255) / 255.0f;
}

// 30 kb = 1500 lines
Mesh::Mesh(const char* path) {
	// temp data
	vector<__m128> tempvertices;
	vector<__m128> temptriangles;
	vector<__m128> tempcolors;
	tempvertices.reserve(1000000);
	temptriangles.reserve(1000000);
	tempcolors.reserve(1000000);

	// reading
	ifstream in(path);
	string token;

	// go until no token is found
	while (in >> token) {
		// read a vertex
		if (token == "v") {
			float x, y, z;
			in >> x >> y >> z;
			tempvertices.push_back(_mm_setr_ps(x, y, z, 1));
		}

		// read a face
		else if (token == "f") {
			string str[3];
			in >> str[0] >> str[1] >> str[2];
			int res[3][3];

			// split '1/2/3'
			for (int i = 0; i < 3; i++) {
				if (str[i].find('/')) {
					// extended
					int s1 = str[i].find('/');
					int s2 = str[i].find('/', s1+1);
					res[i][0] = atoi(str[i].substr(0, s1).c_str()) - 1;
					res[i][1] = atoi(str[i].substr(s1 + 1, s2).c_str()) - 1;
					if (s2 != -1)
						res[i][2] = atoi(str[i].substr(s2 + 1, str[i].size()).c_str()) - 1;
					else
						res[i][2] = 0;
				}
				else {
					// single
					res[i][0] = atoi(str[i].c_str());
					res[i][1] = 0;
					res[i][2] = 0;
				}
			}

			for (int i = 0; i < 3; i++)
				if (res[i][0] < 0 || res[i][0] > tempvertices.size())
					throw "vertex index out of bounds";

			for (int i = 0; i < 3; i++)
				temptriangles.push_back(tempvertices[res[i][0]]);

			tempcolors.push_back(_mm_setr_ps(random1(), random1(), random1(), 1.0f));
		}

		// skip this line
		else {
			getline(in, token);
		}
	}

	// copy to self
	numtriangles = temptriangles.size() / 3;
	triangles = (__m128*)ialloc(temptriangles.size() * 16);
	memcpy(triangles, &temptriangles[0], temptriangles.size() * 16);

	// colors
	numcolors = tempcolors.size();
	colors = (__m128*)ialloc(temptriangles.size() * 16);
	memcpy(colors, &tempcolors[0], tempcolors.size() * 16);
}

Mesh::~Mesh() {
	ifree(triangles);
	ifree(colors);
}