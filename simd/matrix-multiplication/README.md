# Matrix multiplication benchmark for x86\_64

Benchmarking speed of various SIMD based algorithms to calculate matrix 4\*4 multiplication and vector 4 array by matrix 4\*4 multiplication.


## Benchmarks

Note the *ref* was supposed to use simple loop but is optimized by modern compilers too. *novec* is with disabling compiler vectorization, *Fpu87* uses old 8087 FPU on x86\_64.

Benchmarks test single CPU speed, so if there is any resource sharing between CPU cores, this won't be reflected here.

The following was tested:

- matmult: matrix4x4 by matrix4x4 multiplication
- vecmult: vector4 array by row-major matrix4x4 multiplication
- vecTmult: vector4 array by column-major matrix4x4 multiplication

The variants were:

- ref: plain C code, may be vectorized by compiler
- novec: no vectorization applied
- Fpu87: x86\_64 FPU 8087 unit
- Sse: x86\_64 SSE
- SsePar2: x86\_64 SSE, two vectors in parallel
- Avx4Mem: x86\_64 AVX 4 elements at time, memory read
- Avx8: x86\_64 AVX2, 8 elements at time
- Fma: x86\_64 FMA multiply-add, 4 elements at time
- FmaExp: x86\_64 FMA multiply-add expanded, 4 elements at time
- Fma256Exp: x86\_64 FMA multiply-add expanded, 8 elements at time, 2 vectors at time
- Fma256Pre: x86\_64 FMA multiply-add prefetched, 8 elements at time
- Avx512: x86\_64 AVX-512 multiply-add, 16 elements at time, 4 vectors at time
- SseSingles: x86\_64 SSE, 4 elements at time, single vector at time
- Avx256Singles: x86\_64 AVX2, 8 elements at time, single vector at time
- Avx512Singles: x86\_64 AVX-512, 16 elements at time, single vector at time
- TransFma256: x86\_64 FMA multiply-add with transposing matrix first, 8 elements at time, 2 vectors at time
- Neon: aarch64, using multiply-add
- NeonPar2: aarch64, using multiply-add, two vectors in parallel (or two rows of matrix in parallel, as long as compiler is smart enough to interlace)
- Neon (vecTmult): aarch64, using vaddvq\_f32

Observations:

- FPU 8087 unsurprisingly slowest, novec (SSE scalar based) not much faster though.
- AVX2 with 256-bit vectors adds 33% performance if implemented reasonably.
- Column-major matrix looks like bad idea in terms of performance - this effectively results into horizontal add (\_mm\_hadd\_ps on SSE or similar) and permutation and probably resulting into pipeline underutilization.
- With the above, column-major matrix and vector array multiplication could be further optimized to calculate multiple vectors at time, potentially limiting register conflicts.
- As an alternative solution for column-major matrix and vector array multiplication, it is much better to transpose matrix at the beginning and then follow original algorithm.
- For ARM64, vaddvq\_f32 seems to be good enough to eliminate disadvantages of column-major matrix, performing similarly to row-major matrix multiplication.  This may change with SVE or SVE2 though.
- Compiler (depending on version and brand) may require some help in order to expand loops and not to worry about overwriting output and input memory.  In some cases, Par2 versions interlacing vector calculations can give 80% boost.  This could be further optimized manually if separating inputs is not enough for compiler to interlace calculations of two or more vectors.
- Comparing x86\_64 and aarch64, vfmaq\_laneq\_f32 (FMLA with lane instruction) brings benefit which x86\_64 is terribly missing.  Not only it saves instructions but it also saves temporary registers and allows computing full matrix multiplication in just eleven registers while still interlacing the rows in parallel, therefore naturally avoiding execution conflicts.  On the other hand, x86\_64 makes it easy to take operation argument directly from memory with little to no penalty which makes pre-fetching arguments less important.

### Benchmark - Intel(R) Core(TM) i7-5557U CPU @ 3.10GHz :

```
matmult_ref              :  22.49 cycles, avg  24.50 cycles,  126.502 MOPS
matmult_novec            :  64.28 cycles, avg  65.69 cycles,   47.189 MOPS
matmult_Fpu87            :  82.96 cycles, avg  87.01 cycles,   35.626 MOPS
matmult_Sse              :  22.08 cycles, avg  23.03 cycles,  134.590 MOPS
matmult_Avx4Mem          :  19.10 cycles, avg  19.64 cycles,  157.866 MOPS
matmult_Avx8             :  13.25 cycles, avg  13.71 cycles,  226.059 MOPS
matmult_Fma              :  17.83 cycles, avg  18.84 cycles,  164.534 MOPS
matmult_FmaExp           :  18.21 cycles, avg  19.63 cycles,  157.923 MOPS
matmult_Fma256Exp        :  12.30 cycles, avg  13.40 cycles,  231.372 MOPS
matmult_Fma256Pre        :  12.23 cycles, avg  12.91 cycles,  240.038 MOPS
vecmult_ref              :   5.08 cycles, avg   5.42 cycles,  572.174 MOPS
vecmult_novec            :  25.83 cycles, avg  26.52 cycles,  116.891 MOPS
vecmult_Fpu87            :  30.60 cycles, avg  31.24 cycles,   99.216 MOPS
vecmult_Sse              :   4.22 cycles, avg   4.42 cycles,  701.803 MOPS
vecmult_FmaExp           :   4.09 cycles, avg   4.20 cycles,  738.040 MOPS
vecmult_Fma256Exp        :   2.52 cycles, avg   2.62 cycles, 1182.962 MOPS
vecTmult_ref             :  12.77 cycles, avg  12.88 cycles,  240.689 MOPS
vecTmult_SseSingles      :   5.67 cycles, avg   5.85 cycles,  530.069 MOPS
vecTmult_Avx256Singles   :   5.25 cycles, avg   5.53 cycles,  560.282 MOPS
vecTmult_TransFma256     :   2.83 cycles, avg   2.89 cycles, 1073.570 MOPS
```

### Benchmark - Qualcomm Technologies, Inc SDM439 @ 2.016GHz :

```
matmult_ref              : 144.21 cycles, avg 147.32 cycles,   13.554 MOPS
matmult_novec            : 208.20 cycles, avg 215.46 cycles,    9.313 MOPS
matmult_Neon             :  94.99 cycles, avg  98.11 cycles,   20.492 MOPS
matmult_NeonPar2         :  52.17 cycles, avg  54.15 cycles,   37.077 MOPS
vecmult_ref              :  51.93 cycles, avg  54.09 cycles,   37.105 MOPS
vecmult_novec            :  88.16 cycles, avg  89.91 cycles,   22.357 MOPS
vecmult_Neon             :  22.58 cycles, avg  22.67 cycles,   88.722 MOPS
vecmult_NeonPar2         :  15.81 cycles, avg  15.96 cycles,  126.015 MOPS
vecTmult_ref             :  83.12 cycles, avg  83.29 cycles,   24.140 MOPS
vecTmult_Neon            :  21.47 cycles, avg  21.55 cycles,   93.359 MOPS
vecTmult_NeonPar2        :  15.63 cycles, avg  15.90 cycles,  126.531 MOPS
```

### Benchmark - Genuine Intel(R) CPU U7300  @ 1.30GHz :

Ultra low voltage from 2010.

```
matmult_ref              :  63.66 cycles, avg  65.14 cycles,   26.601 MOPS
matmult_novec            : 112.32 cycles, avg 114.32 cycles,   15.158 MOPS
matmult_Fpu87            : 168.19 cycles, avg 170.30 cycles,   10.175 MOPS
matmult_Sse              :  38.87 cycles, avg  40.00 cycles,   43.316 MOPS
vecmult_ref              :  22.55 cycles, avg  22.97 cycles,   75.441 MOPS
vecmult_novec            :  22.41 cycles, avg  22.69 cycles,   76.362 MOPS
vecmult_Fpu87            :  35.89 cycles, avg  36.54 cycles,   47.419 MOPS
vecmult_Sse              :  10.78 cycles, avg  10.92 cycles,  158.621 MOPS
vecTmult_ref             :  22.57 cycles, avg  24.03 cycles,   72.103 MOPS
vecTmult_SseSingles      :  11.96 cycles, avg  12.57 cycles,  137.875 MOPS
```

## License

The code is released under version 2.0 of the [Apache License][].

## Stay in Touch

Feel free to contact me at Zbynek Vyskovsky - kvr000@gmail.com or http://github.com/kvr000 and https://www.linkedin.com/in/zbynek-vyskovsky/ .

[Apache License]: http://www.apache.org/licenses/LICENSE-2.0
