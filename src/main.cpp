#include "config.hpp"
#include "raytrace.hpp"

int main(int argv, char** argc) {
	// load meshname from commandline
	char* meshname = "example.obj";
	if (argv == 2)
		meshname = argc[2];
	Config config("raytrace.cfg");
	Image image(config.RAYTRACE_RES_X, config.RAYTRACE_RES_Y);

	raytrace(&image);

	image.write("output.bmp");
}