#include <iostream>
#include <ctime>
#include <cstdio>
#include "config.hpp"
#include "raytrace.hpp"
#include "mesh.hpp"

int main(int argv, char** argc) {
	// load meshname from commandline
	const char* meshname = "mesh/cube.obj";
	if (argv == 2)
		meshname = argc[2];

	// load objects
	Mesh mesh(meshname);
	Config config("raytrace.cfg");
	Image image(config.RAYTRACE_RES_X, config.RAYTRACE_RES_Y);


	double duration;
    std::clock_t start = std::clock();

	raytrace(mesh, &image);

    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC * 1000; 
    cout<<"Traced in: "<< duration <<" ms\n";

	image.write("output");

	system("pause");
}