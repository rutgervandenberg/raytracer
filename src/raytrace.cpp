#include <ctime> /* clock() */
#include <xmmintrin.h>
#include <cstdlib>
#include <cstdio>
#include "mem.hpp"
#include "raytrace.hpp"
void raytrace(Image* image) {
	// short names
	int w = image->width;
	int h = image->height;

	// make sure aligned on 16 bit boundary
	__m128* rays = (__m128*)ialloc(w * h * 16 * 2);
	__m128* corners = (__m128*)ialloc(16 * 4 * 2);

	clock_t start = clock();

	// corner rays (LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN)
	corners[0] = _mm_setr_ps(0, 0, 0, 1);
	corners[1] = _mm_setr_ps(1, 0, 0, 1);
	corners[2] = _mm_setr_ps(0, 1, 0, 1);
	corners[3] = _mm_setr_ps(1, 1, 0, 1);
	corners[4] = _mm_setr_ps(0, 0, 1, 1);
	corners[5] = _mm_setr_ps(1, 0, 1, 1);
	corners[6] = _mm_setr_ps(0, 1, 1, 1);
	corners[7] = _mm_setr_ps(1, 1, 1, 1);

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
			image->data[y * w + x] = rays[(y * w + x) * 2 + 1];
		}
	}
	
	clock_t ticks = clock() - start;
	printf("%d ms", ticks);
}