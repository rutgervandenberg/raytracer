#pragma once
#include "image.hpp"
#include "mesh.hpp"
#include <xmmintrin.h>

void raytrace(Mesh& mesh, Image* image);
__m128 intersect(__m128 vec, __m128* tr);