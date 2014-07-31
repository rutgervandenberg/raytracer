#pragma once
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <cstring> /* memcpy */

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define SSE

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
//inline vec8 operator<(vec8 a, vec8 b) { return _mm256_cmplt_ps(a, b); }
//inline vec8 operator>=(vec8 a, vec8 b) { return _mm256_cmpge_ps(a, b); }
//inline vec8 operator<=(vec8 a, vec8 b) { return _mm256_cmple_ps(a, b); }
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

// conversions
#define VEC4S(v) _mm_set1_ps(v)
#define VEC4(a,b,c,d) _mm_setr_ps(a,b,c,d)
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
inline vec4 _mm_mod_ps2(vec4 a, vec4 aDiv) {
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

/**
 *	For use when vec4 cannot be a native type
 *	(On ARM & SPARC processors)
 */
struct vec4 {
	float p[4];

	vec4& operator=(float v) {
		p[0] = v; p[1] = v; p[2] = v; p[3] = v;
		return *this;
	}
};

inline vec4 crossP(const vec4 & a, const vec4 & b){
	vec4 result;
	result.p[0] = a.p[1] * b.p[2] - a.p[2] * b.p[1];
	result.p[1] = a.p[2] * b.p[0] - a.p[0] * b.p[2];
	result.p[2] = a.p[0] * b.p[1] - a.p[1] * b.p[0];
	return (result);
}

inline vec4 dot(const vec4 & a, const vec4 & b){
	return (a.p[0] * b.p[0] + a.p[1] * b.p[1] + a.p[2] * b.p[2]);
}

#endif