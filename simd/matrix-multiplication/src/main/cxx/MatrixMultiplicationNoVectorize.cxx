/*
 * Based on existing public code, extended by Zbynek Vyskovsky, kvr000@gmail.com https://github.com/kvr000/zbynek-cxx-exp/
 */

#include <stddef.h>

#include "Math4D.hxx"

// C loop implementation, compiler disabled vectorization
void matmult_novec(Mat44 *out, const Mat44 &A, const Mat44 &B)
{
	Mat44 t;
	for (int i=0; i < 4; i++) {
		for (int j=0; j < 4; j++) {
			t.m[i][j] = A.m[i][0]*B.m[0][j] + A.m[i][1]*B.m[1][j] + A.m[i][2]*B.m[2][j] + A.m[i][3]*B.m[3][j];
		}
	}

	*out = t;
}

// C loop implementation, compiler disabled vectorization
void vecmult_novec(Vector4* out, const Vector4 *in, size_t count, const Mat44 &m)
{
	for (int c = 0; c < count; ++c) {
		Vector4 t;
		for (int j=0; j < 4; j++) {
			t.m[j] = in[c].m[0]*m.m[0][j] + in[c].m[1]*m.m[1][j] + in[c].m[2]*m.m[2][j] + in[c].m[3]*m.m[3][j];
		}

		out[c] = t;
	}
}
