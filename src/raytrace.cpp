#include <ctime> /* clock() */
#include <xmmintrin.h>
#include <cstdlib>
#include <cstdio>
#include "mem.hpp"
#include "raytrace.hpp"

// this is non-optimal as it's individual (?)
/*inline __m128 mmultSSE4(const __m128* matrix, const __m128 vec) {
	__m128 r = _mm_setzero_ps();
	__m128 r1 = _mm_dp_ps(vec, matrix[0], 0x7F);
	__m128 r2 = _mm_dp_ps(vec, matrix[1], 0x7F);
	__m128 r3 = _mm_dp_ps(vec, matrix[2], 0x7F);
	__m128 r4 = _mm_dp_ps(vec, matrix[3], 0x7F);
	r = _mm_blend_ps(r1, r2, 0x2);
	r = _mm_blend_ps(r, r3, 0x4);
	r = _mm_blend_ps(r, r4, 0x8);
	return r;
}*/

inline __m128 mmultSSE2(const __m128* matrix, const __m128 vec) {
	__m128 res;
	__m128 r1, r2;
	__m128 t1, t2, t3, t4;
	r1 = _mm_mul_ps(vec, matrix[0]);
	r2 = _mm_hadd_ps(r1, r1);
	t1 = _mm_hadd_ps(r2, r2);
	r1 = _mm_mul_ps(vec, matrix[1]);
	r2 = _mm_hadd_ps(r1, r1);
	t2 = _mm_hadd_ps(r2, r2);
	r1 = _mm_mul_ps(vec, matrix[2]);
	r2 = _mm_hadd_ps(r1, r1);
	t3 = _mm_hadd_ps(r2, r2);
	r1 = _mm_mul_ps(vec, matrix[3]);
	r2 = _mm_hadd_ps(r1, r1);
	t4 = _mm_hadd_ps(r2, r2);

	__m128 temp1 = _mm_unpacklo_ps(t1, t2);
	__m128 temp2 = _mm_unpackhi_ps(t3, t4);
	res = _mm_shuffle_ps(temp1, temp2, _MM_SHUFFLE(1, 0, 3, 2));
	return res;
}

/* Calculate modulus using vectors */
inline __m128 _mm_mod_ps2(const __m128& a, const __m128& aDiv){
	__m128 c = _mm_div_ps(a, aDiv);
	__m128i i = _mm_cvttps_epi32(c);
	__m128 cTrunc = _mm_cvtepi32_ps(i);
	__m128 base = _mm_mul_ps(cTrunc, aDiv);
	__m128 r = _mm_sub_ps(a, base);
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
	/*matrix[0] = _mm_setr_ps(cos(th),	0,			sin(th),	0);
	matrix[1] = _mm_setr_ps(0,			1,			0,			0);
	matrix[2] = _mm_setr_ps(-sin(th),	0,			cos(th),	0);
	matrix[3] = _mm_setr_ps(0,			0,			0,			1);*/
	matrix[0] = _mm_setr_ps(1, 0, 0, 0);
	matrix[1] = _mm_setr_ps(0, 1, 0, 0);
	matrix[2] = _mm_setr_ps(0, 0, 1, 0);
	matrix[3] = _mm_setr_ps(0, 0, 0, 1);

	// corner rays (LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN)
	corners[0] = _mm_setr_ps(0, 0, 0, 1);
	corners[1] = _mm_setr_ps(1, 0, 0, 1);
	corners[2] = _mm_setr_ps(0, 0, 1, 1);
	corners[3] = _mm_setr_ps(1, 0, 1, 1);
	corners[4] = _mm_setr_ps(0, 1, 0, 1);
	corners[5] = _mm_setr_ps(0, 1, 0, 1);
	corners[6] = _mm_setr_ps(0, 1, 0, 1);
	corners[7] = _mm_setr_ps(0, 1, 0, 1);

	// transform the rays
	for (int i = 0; i < 8; i++)
		corners[i] = mmultSSE2(matrix, corners[i]);

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
	float* val = (float*)ialloc(4 * 4);
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			// pos
			__m128 fac = _mm_div_ps(rays[(y * w + x) * 2 + 0], rays[(y * w + x) * 2 + 1]);
			_mm_store_ps(val, fac);
			float a = val[1];

			__m128 f = _mm_set1_ps(a);
			__m128 impact = _mm_add_ps(rays[(y * w + x) * 2 + 0],
				_mm_mul_ps(f, rays[(y * w + x) * 2 + 1]));

			bool white = true;
			__m128 modulus = _mm_mod_ps2(impact, _mm_set1_ps(0.2f));
			_mm_store_ps(val, modulus);
			if (val[0] > 0.1f)
				white = !white;
			if (val[2] > 0.1f)
				white = !white;

			// set color
			if (white)
				image->data[y * w + x] = _mm_setr_ps(1, 1, 1, 1);
			else
				image->data[y * w + x] = _mm_setr_ps(0, 0, 0, 1);
		}
	}
	ifree(val);
	
	clock_t ticks = clock() - start;
	printf("%lu ms\n", ticks);
}