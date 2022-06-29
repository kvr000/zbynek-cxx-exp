/*
 * Based on existing public code, extended by Zbynek Vyskovsky, kvr000@gmail.com https://github.com/kvr000/zbynek-cxx-exp/
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fstream>
#include <limits>
#include <regex>
#include <functional>
#include <chrono>
#include <ctime>

#ifdef __ARM_FEATURE_SVE
# include <arm_sve.h>
#endif

#include "Math4D.hxx"


void mat_transpose(Mat44 *out, const Mat44 &in)
{
	float f00 = in.m[0][0], f01 = in.m[0][1], f02 = in.m[0][2], f03 = in.m[0][3];
	float f10 = in.m[1][0], f11 = in.m[1][1], f12 = in.m[1][2], f13 = in.m[1][3];
	float f20 = in.m[2][0], f21 = in.m[2][1], f22 = in.m[2][2], f23 = in.m[2][3];
	float f30 = in.m[3][0], f31 = in.m[3][1], f32 = in.m[3][2], f33 = in.m[3][3];

	out->m[0][0] = f00; out->m[0][1] = f10; out->m[0][2] = f20; out->m[0][3] = f30;
	out->m[1][0] = f01; out->m[1][1] = f11; out->m[1][2] = f21; out->m[1][3] = f31;
	out->m[2][0] = f02; out->m[2][1] = f12; out->m[2][2] = f22; out->m[2][3] = f32;
	out->m[3][0] = f03; out->m[3][1] = f13; out->m[3][2] = f23; out->m[3][3] = f33;
}

// C loop implementation (may be vectorized by compiler in newer versions)
void matmult_ref(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	Mat44 t;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			t.m[i][j] = A.m[i][0]*B.m[0][j] + A.m[i][1]*B.m[1][j] + A.m[i][2]*B.m[2][j] + A.m[i][3]*B.m[3][j];
		}
	}

	*out = t;
}

// C loop implementation (may be vectorized by compiler in newer versions)
void vecmult_ref(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m)
{
	for (size_t c = 0; c < count; ++c) {
		Vector4 t;
		for (int j = 0; j < 4; j++) {
			t.m[j] = in[c].m[0]*m.m[0][j] + in[c].m[1]*m.m[1][j] + in[c].m[2]*m.m[2][j] + in[c].m[3]*m.m[3][j];
		}

		out[c] = t;
	}
}

// C loop implementation (may be vectorized by compiler in newer versions)
void vecTmult_ref(Vector4 *out, const Mat44 &m, const Vector4 *in, size_t count)
{
	for (size_t c = 0; c < count; ++c) {
		Vector4 t;
		for (int j = 0; j < 4; j++) {
			t.m[j] = in[c].m[0]*m.m[j][0] + in[c].m[1]*m.m[j][1] + in[c].m[2]*m.m[j][2] + in[c].m[3]*m.m[j][3];
		}

		out[c] = t;
	}
}

void matmult_novec(Mat44 *out, const Mat44 &A, const Mat44 &B);
void vecmult_novec(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m);

#ifdef __x86_64__
void matmult_Fpu87(Mat44 *out, const Mat44 &A, const Mat44 &B);
void vecmult_Fpu87(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m);
#endif

#ifdef __SSE__
// Vector by matrix multiplication, SSE based:
static inline __m128 vectorMultiplyMatrix_Sse(const __m128 a, const Mat44 &B)
{
	__m128 result;
	result = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x00), B.row[0]);
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0x55), B.row[1]));
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xaa), B.row[2]));
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xff), B.row[3]));
	return result;
}

static inline __m128 vectorMultiplyMatrix_Sse(const __m128 a, const __m128 b0, const __m128 b1, const __m128 b2, const __m128 b3)
{
	__m128 result;
	result = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x00), b0);
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0x55), b1));
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xaa), b2));
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xff), b3));
	return result;
}

// SSE based:
void matmult_Sse(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	__m128 out0x = vectorMultiplyMatrix_Sse(A.row[0], B);
	__m128 out1x = vectorMultiplyMatrix_Sse(A.row[1], B);
	__m128 out2x = vectorMultiplyMatrix_Sse(A.row[2], B);
	__m128 out3x = vectorMultiplyMatrix_Sse(A.row[3], B);

	out->row[0] = out0x;
	out->row[1] = out1x;
	out->row[2] = out2x;
	out->row[3] = out3x;
}

void vecmult_Sse(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m)
{
	for (size_t c = 0; c < count; ++c) {
		out[c].row = vectorMultiplyMatrix_Sse(in[c].row, m);
	}
}

void vecmult_SsePar2(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m)
{
	__m128 b0 = m.row[0];
	__m128 b1 = m.row[1];
	__m128 b2 = m.row[2];
	__m128 b3 = m.row[3];

	size_t count0 = count&~1;
	for (size_t c = 0; c < count0; c += 2) {
		__m128 v0 = in[c].row;
		__m128 v1 = in[c+1].row;
		out[c].row = vectorMultiplyMatrix_Sse(v0, b0, b1, b2, b3);
		out[c+1].row = vectorMultiplyMatrix_Sse(v1, b0, b1, b2, b3);
	}
	if ((count&1) != 0) {
		out[count-1].row = vectorMultiplyMatrix_Sse(in[count-1].row, b0, b1, b2, b3);
	}
}

void vecTmult_SseSingles(Vector4 *out, const Mat44 &m, const Vector4 *in, size_t count)
{
	for (size_t c = 0; c < count; c += 1) {
		__m128 v = in[c].row;
		__m128 x0 = _mm_mul_ps(v, m.row[0]);
		__m128 x1 = _mm_mul_ps(v, m.row[1]);
		__m128 x2 = _mm_mul_ps(v, m.row[2]);
		__m128 x3 = _mm_mul_ps(v, m.row[3]);
		__m128 s01 = _mm_hadd_ps(x0, x1);
		__m128 s23 = _mm_hadd_ps(x2, x3);
		__m128 s0123 = _mm_hadd_ps(s01, s23);
		out[c].row = s0123;
	}
}
#endif

#ifdef __AVX__
// vector by matrix multiplication, AVX based:
static inline __m128 vectorMultiplyMatrix_Avx4Mem(const float *a, const Mat44 &B)
{
	__m128 result;
	result = _mm_mul_ps(_mm_broadcast_ss(&a[0]), B.row[0]);
	result = _mm_add_ps(result, _mm_mul_ps(_mm_broadcast_ss(&a[1]), B.row[1]));
	result = _mm_add_ps(result, _mm_mul_ps(_mm_broadcast_ss(&a[2]), B.row[2]));
	result = _mm_add_ps(result, _mm_mul_ps(_mm_broadcast_ss(&a[3]), B.row[3]));
	return result;
}

// AVX based:
void matmult_Avx4Mem(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	_mm256_zeroupper();
	__m128 out0x = vectorMultiplyMatrix_Avx4Mem(A.m[0], B);
	__m128 out1x = vectorMultiplyMatrix_Avx4Mem(A.m[1], B);
	__m128 out2x = vectorMultiplyMatrix_Avx4Mem(A.m[2], B);
	__m128 out3x = vectorMultiplyMatrix_Avx4Mem(A.m[3], B);

	out->row[0] = out0x;
	out->row[1] = out1x;
	out->row[2] = out2x;
	out->row[3] = out3x;
}
#endif

#ifdef __AVX__
// vector by matrix multiplication, AVX based, two at a time:
static inline __m256 vectorMultiplyMatrixDual_Avx(__m256 A01, const Mat44 &B)
{
	__m256 result;
	result = _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x00), _mm256_broadcast_ps(&B.row[0]));
	result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x55), _mm256_broadcast_ps(&B.row[1])));
	result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xaa), _mm256_broadcast_ps(&B.row[2])));
	result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xff), _mm256_broadcast_ps(&B.row[3])));
	return result;
}

// Avxbased, two vectors at once:
void matmult_Avx8(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	__m256 A01 = _mm256_loadu_ps(&A.m[0][0]);
	__m256 A23 = _mm256_loadu_ps(&A.m[2][0]);

	__m256 out01x = vectorMultiplyMatrixDual_Avx(A01, B);
	__m256 out23x = vectorMultiplyMatrixDual_Avx(A23, B);

	_mm256_storeu_ps(&out->m[0][0], out01x);
	_mm256_storeu_ps(&out->m[2][0], out23x);
}

void vecTmult_Avx256Singles(Vector4 *out, const Mat44 &mT, const Vector4 *in, size_t count)
{
	__m256 m02 = _mm256_insertf128_ps(_mm256_castps128_ps256(mT.row[0]), mT.row[2], 1);
	__m256 m13 = _mm256_insertf128_ps(_mm256_castps128_ps256(mT.row[1]), mT.row[3], 1);

	for (size_t c = 0; c < count; c += 1) {
		__m256 v00 = _mm256_broadcast_ps(&in[c].row);
		__m256 r02 = _mm256_mul_ps(v00, m02);
		__m256 r13 = _mm256_mul_ps(v00, m13);
		__m256 s0213 = _mm256_hadd_ps(r02, r13);
		out[c].row = _mm_hadd_ps(_mm256_castps256_ps128(s0213), _mm256_extractf128_ps(s0213, 1));
	}
}
#endif

#ifdef __FMA__
// Vector by matrix multiplication, FMA based:
static inline __m128 vectorMultiplyMatrix_Fma(const __m128 &a, const Mat44 &B)
{
	__m128 result = _mm_mul_ps(_mm_permute_ps(a, 0x00), B.row[0]);
	result = _mm_fmadd_ps(_mm_permute_ps(a, 0x55), B.row[1], result);
	result = _mm_fmadd_ps(_mm_permute_ps(a, 0xaa), B.row[2], result);
	result = _mm_fmadd_ps(_mm_permute_ps(a, 0xff), B.row[3], result);
	return result;
}

// FMA based:
void matmult_Fma(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	__m128 out0x = vectorMultiplyMatrix_Fma(A.row[0], B);
	__m128 out1x = vectorMultiplyMatrix_Fma(A.row[1], B);
	__m128 out2x = vectorMultiplyMatrix_Fma(A.row[2], B);
	__m128 out3x = vectorMultiplyMatrix_Fma(A.row[3], B);

	out->row[0] = out0x;
	out->row[1] = out1x;
	out->row[2] = out2x;
	out->row[3] = out3x;
}
#endif

#ifdef __FMA__
// Vector by matrix multiplication, FMA based:
static inline __m128 vectorMultiplyMatrix_FmaExp(const __m128 a, const __m128 b0, const __m128 b1, const __m128 b2, const __m128 b3)
{
	__m128 result = _mm_mul_ps(_mm_permute_ps(a, 0x00), b0);
	result = _mm_fmadd_ps(_mm_permute_ps(a, 0x55), b1, result);
	result = _mm_fmadd_ps(_mm_permute_ps(a, 0xaa), b2, result);
	result = _mm_fmadd_ps(_mm_permute_ps(a, 0xff), b3, result);
	return result;
}

// FMA based:
void matmult_FmaExp(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	__m128 b0 = B.row[0];
	__m128 b1 = B.row[1];
	__m128 b2 = B.row[2];
	__m128 b3 = B.row[3];

	out->row[0] = vectorMultiplyMatrix_FmaExp(A.row[0], b0, b1, b2, b3);
	out->row[1] = vectorMultiplyMatrix_FmaExp(A.row[1], b0, b1, b2, b3);
	out->row[2] = vectorMultiplyMatrix_FmaExp(A.row[2], b0, b1, b2, b3);
	out->row[3] = vectorMultiplyMatrix_FmaExp(A.row[3], b0, b1, b2, b3);
}

void vecmult_FmaExp(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m)
{
	__m128 b0 = m.row[0];
	__m128 b1 = m.row[1];
	__m128 b2 = m.row[2];
	__m128 b3 = m.row[3];

	for (size_t c = 0; c < count; ++c) {
		out[c].row = vectorMultiplyMatrix_FmaExp(in[c].row, b0, b1, b2, b3);
	}
}
#endif

#ifdef __FMA__
// Vectors by matrix multiplication, FMA256 based:
static inline __m256 vectorMultiplyMatrix_Fma256Exp(const __m256 at, const __m256 b00, const __m256 b11, __m256 b22, __m256 b33)
{
	__m256 result = _mm256_mul_ps(_mm256_permute_ps(at, 0x00), b00);
	result = _mm256_fmadd_ps(_mm256_permute_ps(at, 0x55), b11, result);
	result = _mm256_fmadd_ps(_mm256_permute_ps(at, 0xaa), b22, result);
	result = _mm256_fmadd_ps(_mm256_permute_ps(at, 0xff), b33, result);
	return result;
}

// FMA256 based:
void matmult_Fma256Exp(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	__m256 b00 = _mm256_broadcast_ps(&B.row[0]);
	__m256 b11 = _mm256_broadcast_ps(&B.row[1]);
	__m256 b22 = _mm256_broadcast_ps(&B.row[2]);
	__m256 b33 = _mm256_broadcast_ps(&B.row[3]);

	_mm256_storeu_ps(&out->m[0][0], vectorMultiplyMatrix_Fma256Exp(_mm256_loadu_ps(&A.m[0][0]), b00, b11, b22, b33));
	_mm256_storeu_ps(&out->m[2][0], vectorMultiplyMatrix_Fma256Exp(_mm256_loadu_ps(&A.m[2][0]), b00, b11, b22, b33));
}

// FMA256 based:
void matmult_Fma256Pre(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	// On some CPUs it is better to read at the beginning, to avoid pipeline conflicts
	__m256 a01 = _mm256_loadu_ps(&A.m[0][0]);
	__m256 a23 = _mm256_loadu_ps(&A.m[2][0]);

	__m256 b00 = _mm256_broadcast_ps(&B.row[0]);
	__m256 b11 = _mm256_broadcast_ps(&B.row[1]);
	__m256 b22 = _mm256_broadcast_ps(&B.row[2]);
	__m256 b33 = _mm256_broadcast_ps(&B.row[3]);

	_mm256_storeu_ps(&out->m[0][0], vectorMultiplyMatrix_Fma256Exp(a01, b00, b11, b22, b33));
	_mm256_storeu_ps(&out->m[2][0], vectorMultiplyMatrix_Fma256Exp(a23, b00, b11, b22, b33));
}

void vecmult_Fma256Exp(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m)
{
	__m256 b00 = _mm256_broadcast_ps(&m.row[0]);
	__m256 b11 = _mm256_broadcast_ps(&m.row[1]);
	__m256 b22 = _mm256_broadcast_ps(&m.row[2]);
	__m256 b33 = _mm256_broadcast_ps(&m.row[3]);

	size_t count0 = count&~1;
	for (size_t c = 0; c < count0; c += 2) {
		_mm256_storeu_ps(out[c].m, vectorMultiplyMatrix_Fma256Exp(_mm256_loadu_ps(in[c].m), b00, b11, b22, b33));
	}
	if ((count&1) != 0) {
		out[count-1].row = vectorMultiplyMatrix_FmaExp(in[count-1].row, _mm256_castps256_ps128(b00), _mm256_castps256_ps128(b11), _mm256_castps256_ps128(b22), _mm256_castps256_ps128(b33));
	}
}

void vecTmult_TransFma256(Vector4 *out, const Mat44 &m, const Vector4 *in, size_t count)
{
	__m256 b00, b11, b22, b33;
	{
		__m128 b0 = m.row[0];
		__m128 b1 = m.row[1];
		__m128 b2 = m.row[2];
		__m128 b3 = m.row[3];
		_MM_TRANSPOSE4_PS(b0, b1, b2, b3);
		b00 = _mm256_insertf128_ps(_mm256_castps128_ps256(b0), b0, 1);
		b11 = _mm256_insertf128_ps(_mm256_castps128_ps256(b1), b1, 1);
		b22 = _mm256_insertf128_ps(_mm256_castps128_ps256(b2), b2, 1);
		b33 = _mm256_insertf128_ps(_mm256_castps128_ps256(b3), b3, 1);
	}

	size_t count0 = count&~1;
	for (size_t c = 0; c < count0; c += 2) {
		_mm256_storeu_ps(out[c].m, vectorMultiplyMatrix_Fma256Exp(_mm256_loadu_ps(in[c].m), b00, b11, b22, b33));
	}
	if ((count&1) != 0) {
		out[count-1].row = vectorMultiplyMatrix_FmaExp(in[count-1].row, _mm256_castps256_ps128(b00), _mm256_castps256_ps128(b11), _mm256_castps256_ps128(b22), _mm256_castps256_ps128(b33));
	}
}
#endif

#if (defined __AVX512F__)
// Vectors by matrix multiplication, AVX-512 based:
static inline __m512 vectorMultiplyMatrix_Avx512(const __m512 a0123, const __m512 b0000, const __m512 b1111, __m512 b2222, __m512 b3333)
{
	__m512 result = _mm512_mul_ps(_mm512_permute_ps(a0123, 0x00), b0000);
	result = _mm512_fmadd_ps(_mm512_permute_ps(a0123, 0x55), b1111, result);
	result = _mm512_fmadd_ps(_mm512_permute_ps(a0123, 0xaa), b2222, result);
	result = _mm512_fmadd_ps(_mm512_permute_ps(a0123, 0xff), b3333, result);
	return result;
}

// AVX-512 based:
void matmult_Avx512(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	__m512 a0123 = _mm512_loadu_ps(&A.m[0][0]);
	__m512 b0000 = _mm512_broadcast_f32x4(B.row[0]);
	__m512 b1111 = _mm512_broadcast_f32x4(B.row[1]);
	__m512 b2222 = _mm512_broadcast_f32x4(B.row[2]);
	__m512 b3333 = _mm512_broadcast_f32x4(B.row[3]);

	_mm512_storeu_ps(&out->m[0][0], vectorMultiplyMatrix_Avx512(a0123, b0000, b1111, b2222, b3333));
}

void vecmult_Avx512(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m)
{
	__m512 b0000 = _mm512_broadcast_f32x4(m.row[0]);
	__m512 b1111 = _mm512_broadcast_f32x4(m.row[1]);
	__m512 b2222 = _mm512_broadcast_f32x4(m.row[2]);
	__m512 b3333 = _mm512_broadcast_f32x4(m.row[3]);

	size_t count0 = count&~3;
	for (size_t c = 0; c < count0; c += 4) {
		_mm512_storeu_ps(out[c].m, vectorMultiplyMatrix_Avx512(_mm512_loadu_ps(in[c].m), b0000, b1111, b2222, b3333));
	}
	switch (count&3) {
	case 3:
		_mm256_storeu_ps(out[count0].m, vectorMultiplyMatrix_Fma256Exp(_mm256_loadu_ps(in[count0].m), _mm512_castps512_ps256(b0000), _mm512_castps512_ps256(b1111), _mm512_castps512_ps256(b2222), _mm512_castps512_ps256(b3333)));
		// fall through
	case 1:
		out[count-1].row = vectorMultiplyMatrix_FmaExp(in[count-1].row, _mm512_castps512_ps128(b0000), _mm512_castps512_ps128(b1111), _mm512_castps512_ps128(b2222), _mm512_castps512_ps128(b3333));
		break;
	case 2:
		_mm256_storeu_ps(out[count0].m, vectorMultiplyMatrix_Fma256Exp(_mm256_loadu_ps(in[count0].m), _mm512_castps512_ps256(b0000), _mm512_castps512_ps256(b1111), _mm512_castps512_ps256(b2222), _mm512_castps512_ps256(b3333)));
		break;
	case 0:
		break;
	}
}

void vecTmult_Avx512Singles(Vector4 *out, const Mat44 &m, const Vector4 *in, size_t count)
{
	__m512 a0123 = _mm512_loadu_ps((float *)(uintptr_t)&m.m[0][0]);
	__m512 a0213 = _mm512_insertf32x4(_mm512_insertf32x4(a0123, _mm512_extractf32x4_ps(a0123, 2), 1), _mm512_extractf32x4_ps(a0123, 1), 2);

	for (size_t c = 0; c < count; c += 1) {
		__m512 v0000 = _mm512_broadcast_f32x4(in[c].row);
		__m512 r0213 = _mm512_mul_ps(v0000, a0213);
		__m256 r02 = _mm512_castps512_ps256(r0213);
		__m256 r13 = _mm512_extractf32x8_ps(r0213, 1);
		__m256 s0213 = _mm256_hadd_ps(r02, r13);
		out[c].row = _mm_hadd_ps(_mm256_castps256_ps128(s0213), _mm256_extractf128_ps(s0213, 1));
	}
}
#endif

#ifdef __aarch64__
// Vector by matrix multiplication, Neon based:
static inline float32x4_t vectorMultiplyMatrix_Neon(const float32x4_t a, const float32x4_t b0, const float32x4_t b1, const float32x4_t b2, const float32x4_t b3)
{
	float32x4_t result = vmulq_laneq_f32(b0, a, 0);
	result = vfmaq_laneq_f32(result, b1, a, 1);
	result = vfmaq_laneq_f32(result, b2, a, 2);
	result = vfmaq_laneq_f32(result, b3, a, 3);
	return result;
}

// Neon based:
void matmult_Neon(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	float32x4_t b0 = B.row[0];
	float32x4_t b1 = B.row[1];
	float32x4_t b2 = B.row[2];
	float32x4_t b3 = B.row[3];

	out->row[0] = vectorMultiplyMatrix_Neon(A.row[0], b0, b1, b2, b3);
	out->row[1] = vectorMultiplyMatrix_Neon(A.row[1], b0, b1, b2, b3);
	out->row[2] = vectorMultiplyMatrix_Neon(A.row[2], b0, b1, b2, b3);
	out->row[3] = vectorMultiplyMatrix_Neon(A.row[3], b0, b1, b2, b3);
}

void matmult_NeonPar2(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	float32x4_t b0 = B.row[0];
	float32x4_t b1 = B.row[1];
	float32x4_t b2 = B.row[2];
	float32x4_t b3 = B.row[3];

	float32x4_t a0 = A.row[0];
	float32x4_t a1 = A.row[1];
	float32x4_t a2 = A.row[2];
	float32x4_t a3 = A.row[3];
	out->row[0] = vectorMultiplyMatrix_Neon(a0, b0, b1, b2, b3);
	out->row[1] = vectorMultiplyMatrix_Neon(a1, b0, b1, b2, b3);
	out->row[2] = vectorMultiplyMatrix_Neon(a2, b0, b1, b2, b3);
	out->row[3] = vectorMultiplyMatrix_Neon(a3, b0, b1, b2, b3);
}

void vecmult_Neon(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m)
{
	float32x4_t b0 = m.row[0];
	float32x4_t b1 = m.row[1];
	float32x4_t b2 = m.row[2];
	float32x4_t b3 = m.row[3];

	for (size_t c = 0; c < count; ++c) {
		out[c].row = vectorMultiplyMatrix_Neon(in[c].row, b0, b1, b2, b3);
	}
}

void vecmult_NeonPar2(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m)
{
	float32x4_t b0 = m.row[0];
	float32x4_t b1 = m.row[1];
	float32x4_t b2 = m.row[2];
	float32x4_t b3 = m.row[3];

	size_t count0 = count&~1;
	for (size_t c = 0; c < count0; c += 2) {
		float32x4_t a0 = in[c].row;
		float32x4_t a1 = in[c+1].row;
		float32x4_t r0 = vmulq_laneq_f32(b0, a0, 0);
		float32x4_t r1 = vmulq_laneq_f32(b0, a1, 0);
		r0 = vfmaq_laneq_f32(r0, b1, a0, 1);
		r1 = vfmaq_laneq_f32(r1, b1, a1, 1);
		r0 = vfmaq_laneq_f32(r0, b2, a0, 2);
		r1 = vfmaq_laneq_f32(r1, b2, a1, 2);
		r0 = vfmaq_laneq_f32(r0, b3, a0, 3);
		r1 = vfmaq_laneq_f32(r1, b3, a1, 3);
		out[c].row = r0;
		out[c+1].row = r1;
	}
	if (count&1) {
		out[count-1].row = vectorMultiplyMatrix_Neon(in[count-1].row, b0, b1, b2, b3);
	}
}

void vecTmult_Neon(Vector4 *out, const Mat44 &m, const Vector4 *in, size_t count)
{
	float32x4_t b0 = m.row[0];
	float32x4_t b1 = m.row[1];
	float32x4_t b2 = m.row[2];
	float32x4_t b3 = m.row[3];

	for (size_t c = 0; c < count; ++c) {
		float32x4_t v = in[c].row;
		float32x4_t r0 = vmulq_f32(v, b0);
		float32x4_t r1 = vmulq_f32(v, b1);
		float32x4_t r2 = vmulq_f32(v, b2);
		float32x4_t r3 = vmulq_f32(v, b3);
		float32x4_t s01 = vpaddq_f32(r0, r1);
		float32x4_t s23 = vpaddq_f32(r2, r3);
		float32x4_t s0123 = vpaddq_f32(s01, s23);
		out[c].row = s0123;
	}
}

void vecTmult_NeonPar2(Vector4 *out, const Mat44 &m, const Vector4 *in, size_t count)
{
	float32x4_t b0 = m.row[0];
	float32x4_t b1 = m.row[1];
	float32x4_t b2 = m.row[2];
	float32x4_t b3 = m.row[3];

	size_t count0 = count&~1;
	for (size_t c = 0; c < count0; c += 2) {
		float32x4_t v0 = in[c].row;
		float32x4_t v1 = in[c+1].row;
		float32x4_t v0r0 = vmulq_f32(v0, b0);
		float32x4_t v1r0 = vmulq_f32(v1, b0);
		float32x4_t v0r1 = vmulq_f32(v0, b1);
		float32x4_t v1r1 = vmulq_f32(v1, b1);
		float32x4_t v0r2 = vmulq_f32(v0, b2);
		float32x4_t v1r2 = vmulq_f32(v1, b2);
		float32x4_t v0r3 = vmulq_f32(v0, b3);
		float32x4_t v1r3 = vmulq_f32(v1, b3);
		float32x4_t v0s01 = vpaddq_f32(v0r0, v0r1);
		float32x4_t v1s01 = vpaddq_f32(v1r0, v1r1);
		float32x4_t v0s23 = vpaddq_f32(v0r2, v0r3);
		float32x4_t v1s23 = vpaddq_f32(v1r2, v1r3);
		float32x4_t v0s0123 = vpaddq_f32(v0s01, v0s23);
		float32x4_t v1s0123 = vpaddq_f32(v1s01, v1s23);
		out[c].row = v0s0123;
		out[c+1].row = v1s0123;
	}
}
#endif

#ifdef __ARM_FEATURE_SVE
// SVE based:
void matmult_Sve(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	int n = 4;
	// these are the columns A
	svfloat32_t A0;
	svfloat32_t A1;
	svfloat32_t A2;
	svfloat32_t A3;

	// these are the columns B
	svfloat32_t B0;
	svfloat32_t B1;
	svfloat32_t B2;
	svfloat32_t B3;

	// these are the columns C
	svfloat32_t C0;
	svfloat32_t C1;
	svfloat32_t C2;
	svfloat32_t C3;

	svbool_t pred = svwhilelt_b32_u32(0, n);

	A0 = svld1_f32(pred, &A.row[0][0]);
	A1 = svld1_f32(pred, &A.row[1][0]);
	A2 = svld1_f32(pred, &A.row[2][0]);
	A3 = svld1_f32(pred, &A.row[3][0]);

	// Zero accumulators for C values
	C0 = svdup_n_f32(0);
	C1 = svdup_n_f32(0);
	C2 = svdup_n_f32(0);
	C3 = svdup_n_f32(0);

	// Multiply accumulate in 4x1 blocks, that is each column in C
	B0 = svld1rq_f32(svptrue_b32(), &B.row[0][0]);
	C0 = svmla_lane_f32(C0, A0, B0, 0);
	C0 = svmla_lane_f32(C0, A1, B0, 1);
	C0 = svmla_lane_f32(C0, A2, B0, 2);
	C0 = svmla_lane_f32(C0, A3, B0, 3);
	svst1_f32(pred, &out->row[0][0], C0);    

	B1 = svld1rq_f32(svptrue_b32(), &B.row[1][0]);
	C1 = svmla_lane_f32(C1, A0, B1, 0);
	C1 = svmla_lane_f32(C1, A1, B1, 1);
	C1 = svmla_lane_f32(C1, A2, B1, 2);
	C1 = svmla_lane_f32(C1, A3, B1, 3);
	svst1_f32(pred, &out->row[1][0], C1);

	B2 = svld1rq_f32(svptrue_b32(), &B.row[2][0]);
	C2 = svmla_lane_f32(C2, A0, B2, 0);
	C2 = svmla_lane_f32(C2, A1, B2, 1);
	C2 = svmla_lane_f32(C2, A2, B2, 2);
	C2 = svmla_lane_f32(C2, A3, B2, 3);
	svst1_f32(pred, &out->row[2][0], C2);

	B3 = svld1rq_f32(svptrue_b32(), &B.row[3][0]);
	C3 = svmla_lane_f32(C3, A0, B3, 0);
	C3 = svmla_lane_f32(C3, A1, B3, 1);
	C3 = svmla_lane_f32(C3, A2, B3, 2);
	C3 = svmla_lane_f32(C3, A3, B3, 3);
	svst1_f32(pred, &out->row[3][0], C3);
}
#endif


// ---- testing stuff

static float randf()
{
	return (rand() - 16384.0f) / 1024.0f;
}

static void randmat(Mat44 *M)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			M->m[i][j] = randf();
}

static void randvec(Vector4 *vec)
{
	for (int j = 0; j < 4; j++)
		vec->m[j] = randf();
}

volatile int the_mask = 0; // global volatile to deny optimization

void run_matmult(void (*matmult)(Mat44 *out, const Mat44 &a, const Mat44 &b), Mat44 *out, const Mat44 *A, const Mat44 *B, int count)
{
	for (int i=0; i < count; i++) {
		int j = i & the_mask;
		matmult(&out[j], A[j], B[j]);
	}
}

bool equalsFloat(float l, float r)
{
	if (l == 0) {
		return r-l < std::numeric_limits<float>::min()*1024;
	}
	return abs(r/l-1) < std::numeric_limits<float>::epsilon()*8;
}

bool equalsMatrix(const Mat44 &l, const Mat44 &r)
{
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			if (!equalsFloat(l.m[i][j], r.m[i][j]))
				return false;
		}
	}
	return true;
}

bool equalsVector(const Vector4 &l, const Vector4 &r)
{
	for (size_t j = 0; j < 4; ++j) {
		if (!equalsFloat(l.m[j], r.m[j]))
			return false;
	}
	return true;
}

static uint64_t cpuFrequency;

long readTicks()
{
#if (defined __x86_64__) && 0
	{
		long timer = __rdtsc();
		if (timer != 0) {
			return timer;
		}
	}
#endif
	if (cpuFrequency == 0) {
#ifdef __linux__
		try {
			std::ifstream cpuInfoFd("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
			const std::regex keyValueRegex("^(\\d+(?:\\.\\d*)?)$");
			for (std::string line; getline(cpuInfoFd, line); ) {
				std::smatch match;
				if (std::regex_match(line, match, keyValueRegex)) {
					cpuFrequency = (uint64_t)(stod(match[1], NULL)*1000);
					break;
				}
			}
		}
		catch (...) {
			fprintf(stderr, "Failed to read /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq\n");
		}
#elif __APPLE__

		if (cpuFrequency == 0) {
			FILE *sysctlFd = popen("sysctl hw.cpufrequency", "r");
			if (sysctlFd != NULL) {
				const std::regex keyValueRegex("^([^:]+?)\\s*:\\s*(.*?)\\s*$");
				char buf[1024];
				while (fgets(buf, sizeof(buf)-1, sysctlFd) != NULL) {
					buf[sizeof(buf)-1] = '\0';
					std::smatch match;
					if (std::regex_match(std::string(buf), match, keyValueRegex)) {
						if (match[1] == "hw.cpufrequency") {
							cpuFrequency = (uint64_t)(stod(match[2], NULL));
							break;
						}
					}
				}
				pclose(sysctlFd);
			}
			else {
				fprintf(stderr, "Failed to read sysctl hw.cpufrequency |\n");
			}
		}
		if (cpuFrequency == 0) {
			FILE *powerFd = popen("sudo powermetrics -s cpu_power -n 1", "r");
			if (powerFd != NULL) {
				const std::regex keyValueRegex("^CPU \\d+ active residency:.*(?:\\s+|\\()(\\d+(?:\\.\\d+)?)\\s+MHz:.*\\s*$");
				char buf[1024];
				while (fgets(buf, sizeof(buf)-1, powerFd) != NULL) {
					buf[sizeof(buf)-1] = '\0';
					std::smatch match;
					if (std::regex_match(std::string(buf), match, keyValueRegex)) {
						uint64_t frequency = (uint64_t)(stod(match[1], NULL))*1000000;
						if (frequency > cpuFrequency)
							cpuFrequency = frequency;
					}
				}
				pclose(powerFd);
			}
			else {
				fprintf(stderr, "Failed to read sudo powermetrics -s cpu_power -n 1");
			}
		}
#endif
		if (cpuFrequency != 0) {
			fprintf(stderr, "Found CPU Frequency %.0f\n", (double) cpuFrequency);
		}
		else {
			cpuFrequency = 2500000000;
			fprintf(stderr, "Failed to find CPU frequency, defaulting to %.3f\n", (double) cpuFrequency);
		}
	}
	return clock()*cpuFrequency/CLOCKS_PER_SEC;
}

void runBenchmark(const char *name, long repeatCount, long innerSize, std::function<void()> benchmark)
{
	static const int nruns = 4096;

	unsigned long long best_time = ~0ull;
	unsigned long long start_time = readTicks();
	auto start = std::chrono::system_clock::now();

	for (int run = 0; run < nruns; run++) {
		unsigned long long time = readTicks();
		for (int r = 0; r < repeatCount; ++r) {
			benchmark();
		}
		time = readTicks() - time;
		if (time < best_time)
			best_time = time;
	}
	double avg_time = (double) (readTicks()-start_time) / nruns / repeatCount / innerSize;
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> duration(end-start);

	double cycles_per_run = (double) best_time / repeatCount / innerSize;
	printf("%-25s: %6.2f cycles, avg %6.2f cycles, %8.3f MOPS\n", name, cycles_per_run, avg_time, (double)nruns*repeatCount*innerSize/duration.count()/1000000);
}

// matmult variants
static const struct {
	const char *name;
	void (*matmult)(Mat44 *out, const Mat44 &A, const Mat44 &B);
} matmult_variants[] = {
	{ "matmult_ref",       matmult_ref },
	{ "matmult_novec",     matmult_novec },
#ifdef __x86_64__
	{ "matmult_Fpu87",     matmult_Fpu87 },
#endif
#ifdef __SSE__
	{ "matmult_Sse",       matmult_Sse },
#endif
#ifdef __AVX__
	{ "matmult_Avx4Mem",   matmult_Avx4Mem },
#endif
#ifdef __AVX__
	{ "matmult_Avx8",      matmult_Avx8 },
#endif
#ifdef __FMA__
	{ "matmult_Fma",       matmult_Fma },
#endif
#ifdef __FMA__
	{ "matmult_FmaExp",    matmult_FmaExp },
#endif
#ifdef __FMA__
	{ "matmult_Fma256Exp", matmult_Fma256Exp },
#endif
#ifdef __FMA__
	{ "matmult_Fma256Pre", matmult_Fma256Pre },
#endif
#if (defined __AVX512F__)
	{ "matmult_Avx512",    matmult_Avx512 },
#endif
#if (defined __aarch64__)
	{ "matmult_Neon",      matmult_Neon },
	{ "matmult_NeonPar2",  matmult_NeonPar2 },
#endif
#ifdef __ARM_FEATURE_SVE
	{ "matmult_Sve",       matmult_Sve },
#endif
};

// vecmult variants
static const struct {
	const char *name;
	void (*vecmult)(Vector4 *out, const Vector4 *in, size_t count, const Mat44 &m);
} vecmult_variants[] = {
	{ "vecmult_ref",       vecmult_ref },
	{ "vecmult_novec",     vecmult_novec },
#ifdef __x86_64__
	{ "vecmult_Fpu87",     vecmult_Fpu87 },
#endif
#ifdef __SSE__
	{ "vecmult_Sse",       vecmult_Sse },
	{ "vecmult_SsePar2",   vecmult_SsePar2 },
#endif
#ifdef __FMA__
	{ "vecmult_FmaExp",    vecmult_FmaExp },
#endif
#ifdef __FMA__
	{ "vecmult_Fma256Exp", vecmult_Fma256Exp },
#endif
#if (defined __AVX512F__)
	{ "vecmult_Avx512",    vecmult_Avx512 },
#endif
#if (defined __aarch64__)
	{ "vecmult_Neon",      vecmult_Neon },
	{ "vecmult_NeonPar2",  vecmult_NeonPar2 },
#endif
};

// vecTmult variants
static const struct {
	const char *name;
	void (*vecTmult)(Vector4 *out, const Mat44 &m, const Vector4 *in, size_t count);
} vecTmult_variants[] = {
	{ "vecTmult_ref",           vecTmult_ref },
#ifdef __SSE__
	{ "vecTmult_SseSingles",    vecTmult_SseSingles },
#endif
#ifdef __AVX__
	{ "vecTmult_Avx256Singles", vecTmult_Avx256Singles },
#endif
#ifdef __AVX__
	{ "vecTmult_TransFma256",   vecTmult_TransFma256 },
#endif
#if (defined __AVX512F__)
	{ "vecTmult_Avx512Singles", vecTmult_Avx512Singles },
#endif
#if (defined __aarch64__)
	{ "vecTmult_Neon",          vecTmult_Neon },
	{ "vecTmult_NeonPar2",      vecTmult_NeonPar2 },
#endif
};

int runVerification()
{
	srand(1234); // deterministic random tests

	// matmult correctness tests, all should provide the same result as reference
	// implementation (or close to the same, FMADD may provide better precision)
	for (int i = 0; i < 1000000; i++) {
		Mat44 A, B, AT, BT, out, outT, ref_out, ref_outT;
		randmat(&A);
		randmat(&B);
		mat_transpose(&AT, A);
		mat_transpose(&BT, B);
		matmult_ref(&ref_out, A, B);
		mat_transpose(&ref_outT, ref_out);
		matmult_ref(&outT, BT, AT);
		if (!equalsMatrix(outT, ref_outT)) {
			fprintf(stderr, "transpose mult failed\n");
			return 1;
		}

		for (size_t j = 0; j < sizeof(matmult_variants)/sizeof(matmult_variants[0]); j++) {
			matmult_variants[j].matmult(&out, A, B);
			if (!equalsMatrix(out, ref_out)) {
				fprintf(stderr, "%s failed test %d\n", matmult_variants[j].name, i);
				for (int r = 0; r < 4; ++r) {
					fprintf(stderr, "%15.6f %15.6f %15.6f %15.6f      %15.6f %15.6f %15.6f %15.6f\n", out.m[r][0], out.m[r][1], out.m[r][2], out.m[r][3], ref_out.m[r][0], ref_out.m[r][1], ref_out.m[r][2], ref_out.m[r][3]);
				}
				return 1;
			}
		}
	}

	printf("matmult correctness ok.\n");

	srand(1234); // deterministic random tests

	// vecmult correctness tests, all should provide the same result as reference
	// implementation (or close to the same, FMADD may provide better precision)
	for (int i = 0; i < 100000; i++) {
		Mat44 m, mT;
		Vector4 out[31], in[31], ref_out[31];
		randmat(&m);
		mat_transpose(&mT, m);
		for (size_t c = 0; c < sizeof(in)/sizeof(in[0]); ++c) {
			randvec(&in[c]);
		}
		vecmult_ref(ref_out, in, sizeof(in)/sizeof(in[0]), m);

		for (size_t j = 0; j < sizeof(vecmult_variants)/sizeof(vecmult_variants[0]); j++) {
			vecmult_variants[j].vecmult(out, in, sizeof(in)/sizeof(in[0]), m);
			for (size_t c = 0; c < sizeof(in)/sizeof(in[0]); ++c) {
				if (!equalsVector(out[c], ref_out[c])) {
					fprintf(stderr, "%s failed vecmult test %d\n", vecmult_variants[j].name, i);
					fprintf(stderr, "%15.6f %15.6f %15.6f %15.6f      %15.6f %15.6f %15.6f %15.6f\n", out[c].m[0], out[c].m[1], out[c].m[2], out[c].m[3], ref_out[c].m[0], ref_out[c].m[1], ref_out[c].m[2], ref_out[c].m[3]);
					return 1;
				}
			}
		}
		for (size_t j = 0; j < sizeof(vecTmult_variants)/sizeof(vecTmult_variants[0]); j++) {
			vecTmult_variants[j].vecTmult(out, mT, in, sizeof(in)/sizeof(in[0]));
			for (size_t c = 0; c < sizeof(in)/sizeof(in[0]); ++c) {
				if (!equalsVector(out[c], ref_out[c])) {
					fprintf(stderr, "%s failed vecmult test %d\n", vecTmult_variants[j].name, i);
					fprintf(stderr, "%15.6f %15.6f %15.6f %15.6f      %15.6f %15.6f %15.6f %15.6f\n", out[c].m[0], out[c].m[1], out[c].m[2], out[c].m[3], ref_out[c].m[0], ref_out[c].m[1], ref_out[c].m[2], ref_out[c].m[3]);
					return 1;
				}
			}
		}
	}

	printf("vecmult correctness ok.\n");

	return 0;
}

int runBenchmarkSet()
{
	static const int muls_per_run = 16;

	Mat44 Aperf, ATperf, Bperf, out;
	Vector4 vectors[muls_per_run];
	Vector4 vectorsOut[muls_per_run];
	randmat(&Aperf);
	randmat(&Bperf);
	mat_transpose(&ATperf, Aperf);
	for (size_t i = 0; i < sizeof(vectors)/sizeof(vectors[0]); ++i) {
		randvec(&vectors[i]);
	}

	for (size_t i = 0; i < sizeof(matmult_variants)/sizeof(matmult_variants[0]); i++) {
		runBenchmark(matmult_variants[i].name, 256, muls_per_run, [i, &out, Aperf, Bperf](){ run_matmult(matmult_variants[i].matmult, &out, &Aperf, &Bperf, muls_per_run); });
	}
	for (size_t i = 0; i < sizeof(vecmult_variants)/sizeof(vecmult_variants[0]); i++) {
		runBenchmark(vecmult_variants[i].name, 2048, sizeof(vectors)/sizeof(vectors[0]), [i, vectors, &vectorsOut, Aperf](){ vecmult_variants[i].vecmult(vectorsOut, vectors, sizeof(vectors)/sizeof(vectors[0]), Aperf); });
	}
	for (size_t i = 0; i < sizeof(vecTmult_variants)/sizeof(vecTmult_variants[0]); i++) {
		runBenchmark(vecTmult_variants[i].name, 2048, sizeof(vectors)/sizeof(vectors[0]), [i, vectors, &vectorsOut, ATperf](){ vecTmult_variants[i].vecTmult(vectorsOut, ATperf, vectors, sizeof(vectors)/sizeof(vectors[0])); });
	}
	return 0;
}

int main(int argc, char **argv)
{
	long count = 1;
	if (argc > 1) {
		if ((count = (long) atof(argv[1])) == 0) {
			fprintf(stderr, "Usage: %s [count]\n", argv[0]);
		}
	}
	int err;
	if ((err = runVerification()) != 0) {
		return err;
	}
	for (long i = 0; i < count; ++i) {
		runBenchmarkSet();
	}
	return 0;
}
