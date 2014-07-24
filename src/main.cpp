#include "config.hpp"
#include "raytrace.hpp"
#include "mesh.hpp"

int main(int argv, char** argc) {
	// load meshname from commandline
	const char* meshname = "dragon.obj";
	if (argv == 2)
		meshname = argc[2];

	// load objects
	Mesh mesh(meshname);
	Config config("raytrace.cfg");
	Image image(config.RAYTRACE_RES_X, config.RAYTRACE_RES_Y);

	raytrace(mesh, &image);

	image.write("output.bmp");

	system("pause");
}