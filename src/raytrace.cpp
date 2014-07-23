#include "raytrace.hpp"
#include <ctime> /* clock() */
#include <xmmintrin.h>
#include <cstdlib>

// allocate on 16 byte boundary
void* ialloc(int size) {
	char* ptr = (char*)malloc(size + 16 + 16);
	char* data = (ptr + 16 + 16 - (int)ptr % 16);
	char* info = (char*)data - 16; // 16 bytes in size
	*(char**)info = ptr;
	return data;
}

// free on 16 byte boundary
void ifree(void* ptr) {
	void* real = *(void**) ((char*)ptr - 16);
	free(real);
}

void raytrace(Image& image) {
	// quick 
	int w = image.width;
	int h = image.height;
	// make sure aligned on 16 bit boundary
	__m128* data = (__m128*)ialloc(w * h * 16);
	__m128* rays = (__m128*)ialloc(w * h * 16 * 2);
	__m128* corners = (__m128*)ialloc(16 * 4 * 2);

	clock_t start = clock();

	// corner rays (LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN)
	corners[0] = _mm_setr_ps(0, 0, 0, 1);
	corners[1] = _mm_setr_ps(0, 1, 0, 1);
	corners[2] = _mm_setr_ps(1, 0, 0, 1);
	corners[3] = _mm_setr_ps(1, 1, 0, 1);
	corners[4] = _mm_setr_ps(0, 0, 1, 1);
	corners[5] = _mm_setr_ps(0, 1, 1, 1);
	corners[6] = _mm_setr_ps(1, 0, 1, 1);
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
			__m128 ray2 = _mm_mul_ps(corners[0], val1);
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[1], val2));
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[2], val3));
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[3], val4));

			rays[(y * 640 + x) * 2 + 0] = ray1;
			rays[(y * 640 + x) * 2 + 1] = ray2;
		}
	}

	// fill
	for (int y = 0; y < 480; y++) {
		for (int x = 0; x < 640; x++) {
			data[y * 640 + x] = rays[(y * 640 + x) * 2 + 0];
		}
	}
	
	clock_t ticks = clock() - start;
	println(ticks << " ms");

	// convert
	for (int i = 0; i < 640 * 480; i++){
		float r = data[i].m128_f32[0];
		float g = data[i].m128_f32[1];
		float b = data[i].m128_f32[2];
		float a = data[i].m128_f32[3];
		image.data[i*4+0] = r;
		image.data[i*4+1] = g;
		image.data[i*4+2] = b;
		image.data[i*4+3] = a;
	}

	ifree(data);
}