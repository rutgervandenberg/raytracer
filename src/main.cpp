#include <iostream>
#include <ctime>
#include <cstdio>
#include "config.hpp"
#include "raytrace.hpp"
#include "mesh.hpp"
#include "octree.hpp"

/**
 *	Load the mesh and configuration
 *	then raytrace the image
 *	then save it to a file.
 *	
 *	Bugs:
 *	- x and y are inverted
 *	- it isn't realtime
 */
int main(int argv, char** argc) {
	// load meshname from commandline
	const char* meshname = "mesh/simple-monkey.obj";
	if (argv == 2)
		meshname = argc[2];

	// load objects
	Config conf("raytrace.cfg");
	Mesh mesh(meshname);
	Octree octree(mesh);

	Image image(conf.RAYTRACE_RES_X, conf.RAYTRACE_RES_Y);

	raytrace(conf, mesh, octree, &image);

	image.write("output");

	system("pause");
}