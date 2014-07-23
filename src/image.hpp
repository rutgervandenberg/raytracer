#pragma once
#include <vector>
#include <xmmintrin.h>
using namespace std;

class Image {
public:
	int width;
    int height;
	__m128* data;

	Image();
	Image(int width, int height);
	~Image();
	
	void load(char* path);
    bool write(const char * filename);
};
