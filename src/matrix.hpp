#pragma once
#include "vector.hpp"
#include <cstring> /* memcpy */

/**
 *	Multiply a vector by a 4x4 matrix using only SSE2
 */
inline vec4 mmultSSE2(const vec4* matrix, const vec4 vec) {
	vec4 res;
	vec4 r1, r2;
	vec4 t1, t2, t3, t4;
	r1 = vec * matrix[0];
	r2 = _mm_hadd_ps(r1, r1);
	t1 = _mm_hadd_ps(r2, r2);
	r1 = vec * matrix[1];
	r2 = _mm_hadd_ps(r1, r1);
	t2 = _mm_hadd_ps(r2, r2);
	r1 = vec * matrix[2];
	r2 = _mm_hadd_ps(r1, r1);
	t3 = _mm_hadd_ps(r2, r2);
	r1 = vec * matrix[3];
	r2 = _mm_hadd_ps(r1, r1);
	t4 = _mm_hadd_ps(r2, r2);

	vec4 temp1 = _mm_unpacklo_ps(t1, t2);
	vec4 temp2 = _mm_unpackhi_ps(t3, t4);
	res = _mm_shuffle_ps(temp1, temp2, _MM_SHUFFLE(1, 0, 3, 2));
	return res;
}

/**
 *	Multiply a 4x4 matrix with another matrix
 */
inline void mmultSSE2(vec4* res, const vec4* m1, const vec4* m2) {
	res[0] = mmultSSE2(m1, m2[0]);
	res[1] = mmultSSE2(m1, m2[1]);
	res[2] = mmultSSE2(m1, m2[2]);
	res[3] = mmultSSE2(m1, m2[3]);
}

/**
 *	Transpose a T x T matrix
 *	(this function is not for speed)
 */
template <class T>
void transpose(T* mat) {
	const static int num = sizeof T / 4;
	float temp[num * num];
	float trans[num * num];

	// store
	memcpy(temp, mat, num * num * sizeof (float));

	// transpose
	for (int i = 0; i < num; i++)
	for (int j = 0; j < num; j++)
		trans[i * num + j] = temp[j * 8 + i];

	// load
	memcpy(mat, trans, num * num * sizeof (float));
}