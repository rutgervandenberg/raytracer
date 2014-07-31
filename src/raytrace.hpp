#pragma once
#include "image.hpp"
#include "mesh.hpp"
#include "config.hpp"
#include "octree.hpp"
#include <xmmintrin.h>

void raytrace(const Config& conf, const Mesh& mesh, const Octree& octree, Image* image);