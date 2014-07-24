#include <ctime> /* clock() */
#include <xmmintrin.h>
#include <cstdlib>
#include <cstdio>
#include <cmath> 
#include "mem.hpp"
#include "raytrace.hpp"
#include <pmmintrin.h>
#include <cstring> 
#include <x86intrin.h>

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

inline void mmultSSE2(__m128* res, const __m128* m1, const __m128* m2) {
	res[0] = mmultSSE2(m1, m2[0]);
	res[1] = mmultSSE2(m1, m2[1]);
	res[2] = mmultSSE2(m1, m2[2]);
	res[3] = mmultSSE2(m1, m2[3]);
}

inline void transpose(__m128* res) {
	float dat[4 * 4 * 4];
	memcpy((char*)&dat, (char*)res, 4 * 4 * 4);
	for (int i = 0; i < 4; i++)
		res[i] = _mm_set_ps(dat[0 + i], dat[4 + i], dat[8 + i], dat[1 + i]);
	memcpy((char*)res, (char*)&dat, 4 * 4 * 4);
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

/*inline __m128 operator*(__m128 a, __m128 b) {
	return _mm_mul_ps(a, b);
}*/

__m128 intersect(__m128 vec, __m128* tr) {

	// (float *p, float *d,
	// float *v0, float *v1, float *v2) {

	// float e1[3], e2[3], h[3], s[3], q[3];
	float a, f, u, v;
	__m128 e1 = _mm_sub_ps(tr[1], tr[0]);
	__m128 e2 = _mm_sub_ps(tr[2], tr[0]);

	__m128 h = _mm_mul_ps(vec /* d */, e2);
	// crossProduct(h, d, e2);
	a = innerProduct(e1, h);

	if (a > -0.00001 && a < 0.00001)
		return(false);

	f = 1 / a;
	__m128 s = _mm_sub_ps(vec, tr[0]);
	u = f * (innerProduct(s, h));

	if (u < 0.0 || u > 1.0)
		return(false);

	// crossProduct(q, s, e1);
	__m128 q = _mm_mul_ps(s, e1);
	v = f * innerProduct(d, q);

	if (v < 0.0 || u + v > 1.0)
		return(false);

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	t = f * innerProduct(e2, q);

	if (t > 0.00001) // ray intersection
		return(true);

	else // this means that there is a line intersection
		// but not a ray intersection
		return (false);

}

void raytrace(Mesh& mesh, Image* image) {
	// short names
	int w = image->width;
	int h = image->height;

	// make sure aligned on 16 bit boundary
	__m128* rays = (__m128*)ialloc(w * h * 16 * 2);
	__m128* corners = (__m128*)ialloc(16 * 4);

	// camera position & direction
	__m128 campos = _mm_setr_ps(0, 1, 0, 1);
	__m128 camdir = _mm_setr_ps(1.41f * 0.5f, 1.41f * 0.5f, 0, 1);
	float xdir = 0.0f;
	float ydir = 0.3f;

	// corner rays (LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN)
	corners[0] = _mm_setr_ps(0, 0, 1, 1);
	corners[1] = _mm_setr_ps(1, 0, 1, 1);
	corners[2] = _mm_setr_ps(0, 1, 1, 1);
	corners[3] = _mm_setr_ps(1, 1, 1, 1);

	// rotation matrix
	__m128 rotation[4];
	rotation[0] = _mm_setr_ps(1, 0, 0, 0);
	rotation[1] = _mm_setr_ps(0, cos(ydir), -sin(ydir), 0);
	rotation[2] = _mm_setr_ps(0, sin(ydir), cos(ydir), 0);
	rotation[3] = _mm_setr_ps(0, 0, 0, 1);

	corners[0] = mmultSSE2(rotation, corners[0]);
	corners[1] = mmultSSE2(rotation, corners[1]);
	corners[2] = mmultSSE2(rotation, corners[2]);
	corners[3] = mmultSSE2(rotation, corners[3]);

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
			__m128 ray1 = campos;
			__m128 ray2 = _mm_mul_ps(corners[0], val1);
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[1], val2));
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[2], val3));
			ray2 = _mm_add_ps(ray2, _mm_mul_ps(corners[3], val4));

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
}
