#include <ctime> /* clock() */
#include <xmmintrin.h>
#include <cstdlib>
#include <cstdio>
#include "mem.hpp"
#include "raytrace.hpp"

// this is non-optimal as it's individual
inline __m128 mmult(const __m128* matrix, const __m128 vec) {
	__m128 r = _mm_setzero_ps();
	__m128 r1 = _mm_dp_ps(vec, matrix[0], 0x7F);
	__m128 r2 = _mm_dp_ps(vec, matrix[1], 0x7F);
	__m128 r3 = _mm_dp_ps(vec, matrix[2], 0x7F);
	__m128 r4 = _mm_dp_ps(vec, matrix[3], 0x7F);
	r = _mm_blend_ps(r1, r2, 0x2);
	r = _mm_blend_ps(r, r3, 0x4);
	r = _mm_blend_ps(r, r4, 0x8);
	return r;
}

void raytrace(Mesh& mesh, Image* image) {
	// short names
	int w = image->width;
	int h = image->height;

	// make sure aligned on 16 bit boundary
	__m128* rays = (__m128*)ialloc(w * h * 16 * 2);
	__m128* corners = (__m128*)ialloc(16 * 4 * 2);

	clock_t start = clock();

	// transformation matrix
	__m128 matrix[4];
	float th = 0.9f;
	matrix[0] = _mm_setr_ps(cos(th),	0,			-sin(th),	0);
	matrix[1] = _mm_setr_ps(0,			1,			0,			0);
	matrix[2] = _mm_setr_ps(sin(th),	0,			cos(th),	0);
	matrix[3] = _mm_setr_ps(0,			0,			0,			1);

	// corner rays (LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN)
	corners[0] = _mm_setr_ps(0, 0, 0, 1);
	corners[1] = _mm_setr_ps(1, 0, 0, 1);
	corners[2] = _mm_setr_ps(0, 0, 1, 1);
	corners[3] = _mm_setr_ps(1, 0, 1, 1);
	corners[4] = _mm_setr_ps(-.5f, 1, -.5f, 1);
	corners[5] = _mm_setr_ps(0, 1, 0, 1);
	corners[6] = _mm_setr_ps(0, 1, 0, 1);
	corners[7] = _mm_setr_ps(0, 1, 0, 1);

	// transform the rays
	for (int i = 0; i < 8; i++)
		corners[i] = mmult(matrix, corners[i]);

	// create rays
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			// factors
			float xf2 = (float)x / (float)w;
			float yf2 = (float)y / (float)h;
			float xf1 = 1.0f - xf2;
			float yf1 = 1.0f - yf2;

			// per corner
			float f1 = xf1 * yf1;
			float f2 = xf2 * yf1;
			float f3 = xf1 * yf2;
			float f4 = xf2 * yf2;
			__m128 val1 = _mm_set_ps1(f1);
			__m128 val2 = _mm_set_ps1(f2);
			__m128 val3 = _mm_set_ps1(f3);
			__m128 val4 = _mm_set_ps1(f4);

			// ray is added
			__m128 ray1 = _mm_mul_ps(corners[0], val1);
			ray1 = _mm_add_ps(ray1, _mm_mul_ps(corners[1], val2));
			ray1 = _mm_add_ps(ray1, _mm_mul_ps(corners[2], val3));
			ray1 = _mm_add_ps(ray1, _mm_mul_ps(corners[3], val4));
			__m128 ray2 = _mm_mul_ps(corners[4], val1);
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[5], val2));
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[6], val3));
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[7], val4));

			rays[(y * w + x) * 2 + 0] = ray1;
			rays[(y * w + x) * 2 + 1] = ray2;
		}
	}

	// fill
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			// pos
			float a = rays[(y * w + x) * 2 + 0].m128_f32[1]
				/ rays[(y * w + x) * 2 + 1].m128_f32[1];
			__m128 f = _mm_set1_ps(a);
			__m128 impact = _mm_add_ps(rays[(y * w + x) * 2 + 0],
				_mm_mul_ps(f, rays[(y * w + x) * 2 + 1]));

			bool white = true;
			if (fmodf(impact.m128_f32[0], 0.2f) > 0.1f)
				white = !white;
			if (fmodf(impact.m128_f32[2], 0.2f) > 0.1f)
				white = !white;

			// set color
			if (white)
				image->data[y * w + x] = _mm_setr_ps(1, 1, 1, 1);
			else
				image->data[y * w + x] = _mm_setr_ps(0, 0, 0, 1);
		}
	}
	
	clock_t ticks = clock() - start;
	printf("%lu ms", ticks);
}