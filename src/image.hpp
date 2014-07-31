#pragma once
#include <vector>
#include "vector.hpp"
using namespace std;

class Image {
public:
	int width;
	int height;
	vec4* data;

	Image();
	Image(int width, int height);
	~Image();
	
	void load(char* path);
	void write(const char * filename);
};
