#pragma once
#include "image.hpp"
#include "mesh.hpp"
#include "config.hpp"
#include "octree.hpp"
#include "camera.hpp"

void raytrace(Camera& cam, const Config& conf, const Mesh& mesh, const Octree& octree, Image* image);
void raytrace2(Camera& cam, const Config& conf, const Mesh& mesh, const Octree& octree, Image* image);