#pragma once
#include <string>

class Config {
public:
	Config(std::string configname);
	int MSAA = 0;
	int WINDOW_RES_X = 0;
	int WINDOW_RES_Y = 0;
	int RAYTRACE_RES_X = 0;
	int RAYTRACE_RES_Y = 0;
	int VISIBILITY_TRESHOLD = 0;
	int numcores;
	bool sse;
	bool sse2;
	bool sse3;
	bool sse4;
	bool avx; // 256 bit
	bool avx512;
};
