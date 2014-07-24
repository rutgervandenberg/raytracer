#pragma once
#include <string>

class Config {
public:
	Config(std::string configname);
	int MSAA = 0;
	int WINDOW_RES_X = 640;
	int WINDOW_RES_Y = 480;
	int RAYTRACE_RES_X = WINDOW_RES_X;
	int RAYTRACE_RES_Y = WINDOW_RES_X;
	int VISIBILITY_TRESHOLD = 5;
	int FIELD_OF_VIEW = 75;
	int numcores;
	bool sse;
	bool sse2;
	bool sse3;
	bool sse4;
	bool avx; // 256 bit
	bool avx512;
};
