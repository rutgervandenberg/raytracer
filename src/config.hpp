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
};
