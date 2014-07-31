#pragma once
#define SSE

#ifdef SSE
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <cstring> /* memcpy */

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#endif


/**
 * THIS FILE CONTAINS VECTOR MATH
 * Include it in every file containing vector intrinsics!
 */

// when using SSE
#ifdef SSE

typedef __m128 vec4;
typedef __m256 vec8;
typedef __m128i ivec4;
typedef __m256i ivec8;

// common operators (for windows)
#ifdef _WIN32

// 128 bits
inline vec4 operator+(vec4 a, vec4 b) { return _mm_add_ps(a, b); }
inline vec4 operator-(vec4 a, vec4 b) { return _mm_sub_ps(a, b); }
inline vec4 operator*(vec4 a, vec4 b) { return _mm_mul_ps(a, b); }
inline vec4 operator/(vec4 a, vec4 b) { return _mm_div_ps(a, b); }
inline vec4 operator>(vec4 a, vec4 b) { return _mm_cmpgt_ps(a, b); }
inline vec4 operator<(vec4 a, vec4 b) { return _mm_cmplt_ps(a, b); }
inline vec4 operator>=(vec4 a, vec4 b) { return _mm_cmpge_ps(a, b); }
inline vec4 operator<=(vec4 a, vec4 b) { return _mm_cmple_ps(a, b); }
inline vec4 operator&(vec4 a, vec4 b) { return _mm_and_ps(a, b); }
inline vec4 operator|(vec4 a, vec4 b) { return _mm_or_ps(a, b); }
inline vec4 operator^(vec4 a, vec4 b) { return _mm_xor_ps(a, b); }

inline vec4 operator!(vec4 a) {
	const unsigned i = 0xFFFFFFFF;
	return _mm_xor_ps(_mm_set1_ps(*(float*)&i), a);
}
inline vec4 sqrt(vec4 a) { return _mm_sqrt_ps(a); }
inline vec4 max(vec4 a, vec4 b) { return _mm_max_ps(a, b); }
inline vec4 min(vec4 a, vec4 b) { return _mm_min_ps(a, b); }

// 256 bits
inline vec8 operator+(vec8 a, vec8 b) { return _mm256_add_ps(a, b); }
inline vec8 operator-(vec8 a, vec8 b) { return _mm256_sub_ps(a, b); }
inline vec8 operator*(vec8 a, vec8 b) { return _mm256_mul_ps(a, b); }
inline vec8 operator/(vec8 a, vec8 b) { return _mm256_div_ps(a, b); }
inline vec8 operator>(vec8 a, vec8 b) { return _mm256_cmp_ps(a, b, _CMP_GT_OQ); }
inline vec8 operator<(vec8 a, vec8 b) { return _mm256_cmp_ps(a, b, _CMP_LT_OQ); }
inline vec8 operator>=(vec8 a, vec8 b) { return _mm256_cmp_ps(a, b, _CMP_GE_OQ); }
inline vec8 operator<=(vec8 a, vec8 b) { return _mm256_cmp_ps(a, b, _CMP_LE_OQ); }
inline vec8 operator&(vec8 a, vec8 b) { return _mm256_and_ps(a, b); }
inline vec8 operator|(vec8 a, vec8 b) { return _mm256_or_ps(a, b); }
inline vec8 operator^(vec8 a, vec8 b) { return _mm256_xor_ps(a, b); }

inline vec8 operator!(vec8 a) {
	const unsigned i = 0xFFFFFFFF;
	return _mm256_xor_ps(_mm256_set1_ps(*(float*)&i), a);
}

inline vec8 sqrt(vec8 a) { return _mm256_sqrt_ps(a); }
#endif

// conversion functions
template <typename A, typename B>
B convert(A a) { return (B)a; }

template <>
inline vec8 convert<float, vec8>(float a) { return _mm256_set1_ps(a); }

template <>
inline vec4 convert<float,vec4>(float a) { return _mm_set1_ps(a); }

inline float get0(const vec4& a) { int i = _mm_extract_ps(a, 0); return *(float*)&i; }
inline float get1(const vec4& a) { int i = _mm_extract_ps(a, 1); return *(float*)&i; }
inline float get2(const vec4& a) { int i = _mm_extract_ps(a, 2); return *(float*)&i; }
inline float get3(const vec4& a) { int i = _mm_extract_ps(a, 3); return *(float*)&i; }
inline float get(const vec4& a, int idx) { return a.m128_f32[idx]; }// float temp[4];  _mm_store_ps(temp, a); return temp[idx]; }

// init
#define vec4_0() _mm_setzero_ps()
#define vec4s(v) _mm_set1_ps(v)
#define vec4(a,b,c,d) _mm_setr_ps(a,b,c,d)
#define ZERO4() _mm_setzero_ps()

// advanced functions
#define cross _mm2_cp_ps
#define dot _mm_dp_ps

/* Calculate dot without using SSE4 */
inline vec4 _mm2_dp_ps(vec4 a, vec4 b, int un) {
	vec4 c = a * b;
	c = _mm_hadd_ps(c, c);
	c = _mm_hadd_ps(c, c);
	return c;
}

/* Calculate modulus using vectors */
inline vec4 _mm2_mod_ps2(vec4 a, vec4 aDiv) {
	vec4 c = a / aDiv;
	ivec4 i = _mm_cvttps_epi32(c);
	i = _mm_abs_epi32(i);
	vec4 cTrunc = _mm_cvtepi32_ps(i);
	vec4 base = cTrunc * aDiv;
	vec4 r = a - base;
	return r;
}

/**
 *	Calculate dot product with SSE2
 */
inline vec4 _mm2_cp_ps(vec4 a, vec4 b)
{
	vec4 a1 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
	vec4 a2 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2));
	vec4 b1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
	vec4 b2 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2));
	return a1 * b2 - a2 * b1;
}

#else

const unsigned izeroes = 0x00000000;
const unsigned iones = 0xFFFFFFFF;
const float zeroes = *(float*)&izeroes;
const float ones = *(float*)&iones;

/**
 *	For use when vec4 cannot be a native type
 *	(On ARM & SPARC processors)
 */
template <typename T>
struct tvec4 {
	T p[4];

	tvec4() { p[0] = 0; p[1] = 0; p[2] = 0; p[3] = 0; }
	tvec4(float v) { p[0] = v; p[1] = v; p[2] = v; p[3] = v; }
	tvec4(float a, float b, float c) { p[0] = a; p[1] = b; p[2] = c; p[3] = 0; }
	tvec4(float a, float b, float c, float d) { p[0] = a; p[1] = b; p[2] = c; p[3] = d;}

	const float& operator[](const int i) const { return p[i]; }
	float& operator[](const int i) { return p[i]; }
	tvec4 operator+(const tvec4 b) const { return tvec4(p[0]+b[0], p[1]+b[1], p[2]+b[2]); }
	tvec4 operator-(const tvec4 b) const { return tvec4(p[0]-b[0], p[1]-b[1], p[2]-b[2]); }
	tvec4 operator*(const tvec4 b) const { return tvec4(p[0]*b[0], p[1]*b[1], p[2]*b[2]); }
	tvec4 operator/(const tvec4 b) const { return tvec4(p[0] / b[0], p[1] / b[1], p[2] / b[2]); }
	tvec4 operator&(const tvec4 b) const {
		int* ia = (int*)p;
		int* ib = (int*)b.p;
		return tvec4(ia[0] & ib[0], ia[1] & ib[1], ia[2] & ib[2], ia[3] & ib[3]);
	}
	tvec4 operator|(const tvec4 b) const {
		int* ia = (int*)p;
		int* ib = (int*)b.p;
		return tvec4(ia[0] | ib[0], ia[1] | ib[1], ia[2] | ib[2], ia[3] | ib[3]);
	}
	tvec4 operator>(const tvec4 b) const {
		int* ia = (int*)p;
		int* ib = (int*)b.p;
		float f1 = p[0] > b[0] ? ones : zeroes;
		float f2 = p[1] > b[1] ? ones : zeroes;
		float f3 = p[2] > b[2] ? ones : zeroes;
		float f4 = p[3] > b[3] ? ones : zeroes;
		return tvec4(f1, f2, f3, f4);
	}
	tvec4 operator<(const tvec4 b) const {
		int* ia = (int*)p;
		int* ib = (int*)b.p;
		float f1 = p[0] < b[0] ? ones : zeroes;
		float f2 = p[1] < b[1] ? ones : zeroes;
		float f3 = p[2] < b[2] ? ones : zeroes;
		float f4 = p[3] < b[3] ? ones : zeroes;
		return tvec4(f1, f2, f3, f4);
	}
	tvec4 operator>=(tvec4 b) {
		int* ia = (int*)p;
		int* ib = (int*)b.p;
		float f1 = p[0] >= b[0] ? ones : zeroes;
		float f2 = p[1] >= b[1] ? ones : zeroes;
		float f3 = p[2] >= b[2] ? ones : zeroes;
		float f4 = p[3] >= b[3] ? ones : zeroes;
		return tvec4(f1, f2, f3, f4);
	}
	tvec4 operator<=(tvec4 b) {
		int* ia = (int*)p;
		int* ib = (int*)b.p;
		float f1 = p[0] <= b[0] ? ones : zeroes;
		float f2 = p[1] <= b[1] ? ones : zeroes;
		float f3 = p[2] <= b[2] ? ones : zeroes;
		float f4 = p[3] <= b[3] ? ones : zeroes;
		return tvec4(f1, f2, f3, f4);
	}
};

typedef tvec4<float> vec4;
typedef tvec4<int> ivec4;

inline vec4 vec4s(float v) { return vec4(v); }
inline float get0(const vec4& a) { return a[0]; }
inline float get1(const vec4& a) { return a[1]; }
inline float get2(const vec4& a) { return a[2]; }
inline float get3(const vec4& a) { return a[3]; }
inline float get(const vec4& a, int idx) { return a[idx]; }

inline vec4 sqrt(vec4 v) {
	return vec4(sqrt(v[0]), sqrt(v[1]), sqrt(v[2]), sqrt(v[3]));
}

inline vec4 cross(const vec4 & a, const vec4 & b) {
	vec4 result;
	result.p[0] = a.p[1] * b.p[2] - a.p[2] * b.p[1];
	result.p[1] = a.p[2] * b.p[0] - a.p[0] * b.p[2];
	result.p[2] = a.p[0] * b.p[1] - a.p[1] * b.p[0];
	return (result);
}

inline vec4 dot(const vec4 & a, const vec4 & b, int bogus) {
	return vec4(a.p[0] * b.p[0] + a.p[1] * b.p[1] + a.p[2] * b.p[2]);
}

inline vec4 max(vec4 a, vec4 b) {
	return vec4(fmax(a[0], b[0]), fmax(a[1], b[1]), fmax(a[2], b[2]), fmax(a[3], b[3]));
}

inline vec4 min(vec4 a, vec4 b) {
	return vec4(fmin(a[0], b[0]), fmin(a[1], b[1]), fmin(a[2], b[2]), fmin(a[3], b[3]));
}

inline vec4 _mm_hadd_ps(vec4 a, vec4 b) {
	return vec4(a[0] + a[1], a[2] + a[3], b[0] + b[1], b[2] + b[3]);
}

inline vec4 _mm_unpacklo_ps(vec4 a, vec4 b) {
	return vec4(a[0], b[0], a[1], b[1]);
}

inline vec4 _mm_unpackhi_ps(vec4 a, vec4 b) {
	return vec4(a[2], b[2], a[3], b[3]);
}

#define _MM_SHUFFLE(z,y,x,w) (z << 6) | (y << 4) | (x << 2) | w

inline vec4 _mm_shuffle_ps(vec4 a, vec4 b, int imm8) {
	vec4 res;
	int idx1 = (imm8 >> 0) & 0x3;
	int idx2 = (imm8 >> 2) & 0x3;
	int idx3 = (imm8 >> 4) & 0x3;
	int idx4 = (imm8 >> 6) & 0x3;
	res[0] = a[idx1];
	res[1] = a[idx2];
	res[2] = b[idx1];
	res[3] = b[idx2];
	return res;
}

inline void _mm_store_ps(float* v, vec4 a) {
	memcpy(v, a.p, 16);
}

inline void _mm_load_ps(float* v, vec4 a) {
	memcpy(a.p, v, 16);
}

inline void _mm_store_ss(float* v, vec4 a) {
	memcpy(a.p, v, 4);
}
#endif