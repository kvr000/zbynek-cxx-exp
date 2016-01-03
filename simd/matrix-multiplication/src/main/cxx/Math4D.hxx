/*
 * Based on existing public code, extended by Zbynek Vyskovsky, kvr000@gmail.com https://github.com/kvr000/zbynek-cxx-exp/
 */

#ifndef Math4D_hxx__
# define Math4D_hxx__

#ifndef NO_VECTORIZE
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif
#ifdef __aarch64__
#include <arm_neon.h>
#endif
#endif


union Mat44 {
	float m[4][4];
#ifndef NO_VECTORIZE
#ifdef __x86_64__
	__m128 row[4];
	__m256 rowDuet[2];
	__m512 rowQuad;
#endif
#ifdef __aarch64__
	float32x4_t row[4];
#endif
#endif
};

union Vector4 {
	float m[4];
#ifndef NO_VECTORIZE
#ifdef __x86_64__
	__m128 row;
#endif
#ifdef __aarch64__
	float32x4_t row;
#endif
#endif
};


#endif
