# Matrix multiplication benchmark for various architectures

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
- NeonPar4: aarch64, using multiply-add, four vectors in parallel (or four rows of matrix in parallel, explicitly)
- Neon (vecTmult): aarch64, using vaddvq\_f32
- SveRows: aarch64, matrix multiplication based on per-row multiplication.
- SveSingle: aarch64, matrix multiplication based on all rows at once.
- Sve (vecmult): aarch64, vector multiplication, all array elements at once.

Observations:

- FPU 8087 unsurprisingly slowest, novec (SSE scalar based) not much faster though.
- AVX2 with 256-bit vectors adds 33% performance if implemented reasonably.
- Column-major matrix looks like bad idea in terms of performance - this effectively results into horizontal add (\_mm\_hadd\_ps on SSE or similar) and permutation and probably resulting into pipeline underutilization.
- With the above, column-major matrix and vector array multiplication could be further optimized to calculate multiple vectors at time, potentially limiting register conflicts.
- As an alternative solution for column-major matrix and vector array multiplication, it is much better to transpose matrix at the beginning and then follow original algorithm.
- For ARM64, vaddvq\_f32 seems to be good enough to eliminate disadvantages of column-major matrix, performing similarly to row-major matrix multiplication.  This may change with SVE or SVE2 though.
- Compiler (depending on version and brand) may require some help in order to expand loops and not to worry about overwriting output and input memory.  In some cases, Par2 versions interlacing vector calculations can give 80% boost.  This could be further optimized manually if separating inputs is not enough for compiler to interlace calculations of two or more vectors.
- Comparing x86\_64 and aarch64, vfmaq\_laneq\_f32 (FMLA with lane instruction) brings benefit which x86\_64 is terribly missing.  Not only it saves instructions but it also saves temporary registers and allows computing full matrix multiplication in just eleven registers while still interlacing the rows in parallel, therefore naturally avoiding execution conflicts.  On the other hand, x86\_64 makes it easy to take operation argument directly from memory with little to no penalty which makes pre-fetching arguments less important.
- aarch64 by desktop Apple M1 Pro is narrow winner among all measured CPUs (except SVE, see later) while aarch64 by server Amazon Graviton2 is actually 2.5 times slower per clock.  Comparing ref and novec, it seems M1 Pro is much better in SIMD parallelization than Graviton2.
- aarch64 tested CPUs kept the performance consistently when tested on multiple cores for long time while the x86\_64 tested CPUs slowed down quickly after CPU got overheated (this is well known problem especially with AVX-512).
- aarch64 SVE2 extension makes vector array multiplication impressively fast - only 0.26 clock, in comparison to Neon where it takes 1.62 clock.  Matrix multiplication is faster too, though only by 40% as it's single operation only.  The best and average numbers differ by 20% though and even more for matrix multiplication.

### Benchmark - Genuine Intel(R) CPU U7300  @ 1.30GHz

Laptop ultra low voltage x86_64 2010.

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

### Benchmark - Intel(R) Core(TM) i7-5557U CPU @ 3.10GHz

Laptop x86_64.

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

### Benchmark - Intel(R) Xeon(R) Platinum 8275CL CPU @ 3.00GHz

Server x86_64.

```
matmult_ref              :  15.84 cycles, avg  15.94 cycles,  188.207 MOPS
matmult_novec            :  56.88 cycles, avg  57.03 cycles,   52.603 MOPS
matmult_Fpu87            :  85.14 cycles, avg  85.98 cycles,   34.893 MOPS
matmult_Sse              :  15.92 cycles, avg  16.06 cycles,  186.811 MOPS
matmult_Avx4Mem          :  13.52 cycles, avg  13.62 cycles,  220.327 MOPS
matmult_Avx8             :  10.38 cycles, avg  10.43 cycles,  287.501 MOPS
matmult_Fma              :  14.41 cycles, avg  14.60 cycles,  205.510 MOPS
matmult_FmaExp           :  14.69 cycles, avg  14.81 cycles,  202.552 MOPS
matmult_Fma256Exp        :  10.36 cycles, avg  10.42 cycles,  288.015 MOPS
matmult_Fma256Pre        :  10.36 cycles, avg  10.46 cycles,  286.791 MOPS
matmult_Avx512           :   8.01 cycles, avg   8.06 cycles,  372.089 MOPS
vecmult_ref              :   2.98 cycles, avg   3.01 cycles,  995.246 MOPS
vecmult_novec            :   7.58 cycles, avg   7.60 cycles,  394.521 MOPS
vecmult_Fpu87            :  15.38 cycles, avg  15.83 cycles,  189.478 MOPS
vecmult_Sse              :   3.68 cycles, avg   3.71 cycles,  809.488 MOPS
vecmult_SsePar2          :   4.03 cycles, avg   4.04 cycles,  742.972 MOPS
vecmult_FmaExp           :   3.09 cycles, avg   3.10 cycles,  968.051 MOPS
vecmult_Fma256Exp        :   2.10 cycles, avg   2.11 cycles, 1420.621 MOPS
vecmult_Avx512           :   1.44 cycles, avg   1.47 cycles, 2041.647 MOPS
vecTmult_ref             :   3.09 cycles, avg   3.10 cycles,  966.410 MOPS
vecTmult_SseSingles      :   5.30 cycles, avg   5.33 cycles,  562.521 MOPS
vecTmult_Avx256Singles   :   4.74 cycles, avg   4.80 cycles,  624.919 MOPS
vecTmult_TransFma256     :   2.67 cycles, avg   2.68 cycles, 1118.091 MOPS
vecTmult_Avx512Singles   :   6.03 cycles, avg   6.29 cycles,  477.300 MOPS
```

### Benchmark - AMD EPYC 7R32 @ 2.8GHz

Server x86_64.

```
matmult_ref              :  13.71 cycles, avg  14.27 cycles,  196.140 MOPS
matmult_novec            :  54.74 cycles, avg  56.92 cycles,   49.190 MOPS
matmult_Fpu87            :  98.33 cycles, avg 100.74 cycles,   27.794 MOPS
matmult_Sse              :  13.54 cycles, avg  14.05 cycles,  199.242 MOPS
matmult_Avx4Mem          :  16.02 cycles, avg  16.44 cycles,  170.261 MOPS
matmult_Avx8             :   9.69 cycles, avg  10.28 cycles,  272.244 MOPS
matmult_Fma              :  14.44 cycles, avg  15.34 cycles,  182.527 MOPS
matmult_FmaExp           :  14.27 cycles, avg  14.65 cycles,  191.113 MOPS
matmult_Fma256Exp        :  10.87 cycles, avg  11.22 cycles,  249.466 MOPS
matmult_Fma256Pre        :  10.69 cycles, avg  11.24 cycles,  249.078 MOPS
vecmult_ref              :   2.42 cycles, avg   2.48 cycles, 1128.754 MOPS
vecmult_novec            :   9.00 cycles, avg   9.17 cycles,  305.380 MOPS
vecmult_Fpu87            :  24.12 cycles, avg  24.54 cycles,  114.075 MOPS
vecmult_Sse              :   3.38 cycles, avg   3.42 cycles,  818.452 MOPS
vecmult_SsePar2          :   3.37 cycles, avg   3.41 cycles,  821.868 MOPS
vecmult_FmaExp           :   2.75 cycles, avg   2.87 cycles,  974.379 MOPS
vecmult_Fma256Exp        :   1.94 cycles, avg   2.00 cycles, 1399.290 MOPS
vecTmult_ref             :   2.39 cycles, avg   2.41 cycles, 1161.737 MOPS
vecTmult_SseSingles      :   5.65 cycles, avg   5.73 cycles,  488.236 MOPS
vecTmult_Avx256Singles   :   4.79 cycles, avg   4.88 cycles,  573.806 MOPS
vecTmult_TransFma256     :   2.10 cycles, avg   2.18 cycles, 1286.653 MOPS
```

### Benchmark - Intel(R) Core(TM) i7-1185G7 @ 3.00GHz

2020 laptop x86_64.

Tested with clang version 13.0.0-2

```
matmult_ref              :  22.27 cycles, avg  24.28 cycles,  197.229 MOPS
matmult_novec            :  57.42 cycles, avg  58.07 cycles,   82.652 MOPS
matmult_Fpu87            :  75.00 cycles, avg  90.17 cycles,   53.231 MOPS
matmult_Sse              :  15.23 cycles, avg  16.39 cycles,  292.763 MOPS
matmult_Avx4Mem          :  15.23 cycles, avg  16.59 cycles,  288.668 MOPS
matmult_Avx8             :   9.38 cycles, avg  10.97 cycles,  437.376 MOPS
matmult_Fma              :  15.23 cycles, avg  16.02 cycles,  299.671 MOPS
matmult_FmaExp           :  14.06 cycles, avg  16.85 cycles,  284.885 MOPS
matmult_Fma256Exp        :   8.20 cycles, avg   9.51 cycles,  504.978 MOPS
matmult_Fma256Pre        :   8.20 cycles, avg   9.48 cycles,  506.191 MOPS
matmult_Avx512           :   7.03 cycles, avg   7.66 cycles,  626.281 MOPS
vecmult_ref              :   3.96 cycles, avg   4.15 cycles, 1156.402 MOPS
vecmult_novec            :  14.36 cycles, avg  14.54 cycles,  330.136 MOPS
vecmult_Fpu87            :  22.85 cycles, avg  23.10 cycles,  207.771 MOPS
vecmult_Sse              :   5.57 cycles, avg   5.81 cycles,  825.581 MOPS
vecmult_SsePar2          :   3.52 cycles, avg   3.87 cycles, 1241.249 MOPS
vecmult_FmaExp           :   2.93 cycles, avg   3.15 cycles, 1522.511 MOPS
vecmult_Fma256Exp        :   2.05 cycles, avg   2.26 cycles, 2124.546 MOPS
vecmult_Avx512           :   1.46 cycles, avg   1.69 cycles, 2846.387 MOPS
vecTmult_ref             :   3.96 cycles, avg   4.21 cycles, 1140.885 MOPS
vecTmult_SseSingles      :   6.15 cycles, avg   6.42 cycles,  747.530 MOPS
vecTmult_Avx256Singles   :   5.13 cycles, avg   5.32 cycles,  902.880 MOPS
vecTmult_TransFma256     :   2.78 cycles, avg   2.99 cycles, 1607.395 MOPS
vecTmult_Avx512Singles   :   6.45 cycles, avg   6.67 cycles,  719.699 MOPS
```

### Benchmark - Qualcomm Technologies, Inc SDM439 @ 2.016GHz

Mobile aarch64.

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

### Benchmark - Graviton2 :

Server aarch64.

```
matmult_ref              :  37.84 cycles, avg  38.37 cycles,   65.017 MOPS
matmult_novec            :  51.88 cycles, avg  53.21 cycles,   46.966 MOPS
matmult_Neon             :  13.43 cycles, avg  14.19 cycles,  176.208 MOPS
matmult_NeonPar2         :  13.43 cycles, avg  14.18 cycles,  176.331 MOPS
vecmult_ref              :   7.02 cycles, avg   7.11 cycles,  351.793 MOPS
vecmult_novec            :  10.60 cycles, avg  10.69 cycles,  233.958 MOPS
vecmult_Neon             :   3.51 cycles, avg   3.56 cycles,  702.897 MOPS
vecmult_NeonPar2         :   3.36 cycles, avg   3.46 cycles,  721.872 MOPS
vecTmult_ref             :   6.94 cycles, avg   6.98 cycles,  358.082 MOPS
vecTmult_Neon            :   4.88 cycles, avg   4.92 cycles,  508.234 MOPS
vecTmult_NeonPar2        :   4.81 cycles, avg   4.93 cycles,  507.268 MOPS
```

### Benchmark - Graviton3 :

Server aarch64.  Performance less stable, not sure whether because of competing threads in cloud environments or overheating.

```
matmult_ref              :  21.97 cycles, avg  22.67 cycles,  110.261 MOPS
matmult_novec            :  34.18 cycles, avg  34.81 cycles,   71.806 MOPS
matmult_Neon             :   7.93 cycles, avg   9.36 cycles,  267.163 MOPS
matmult_NeonPar2         :   7.93 cycles, avg   9.35 cycles,  267.375 MOPS
matmult_SveRows          :  10.38 cycles, avg  15.37 cycles,  169.186 MOPS
matmult_SveSingle        :   4.44 cycles, avg   7.04 cycles,  369.560 MOPS
vecmult_ref              :   3.28 cycles, avg   3.86 cycles,  647.590 MOPS
vecmult_novec            :   6.56 cycles, avg   6.73 cycles,  371.362 MOPS
vecmult_Neon             :   1.91 cycles, avg   2.30 cycles, 1087.943 MOPS
vecmult_NeonPar2         :   1.91 cycles, avg   2.27 cycles, 1102.215 MOPS
vecmult_Sve              :   0.40 cycles, avg   0.48 cycles, 5411.007 MOPS
vecTmult_ref             :   3.89 cycles, avg   4.49 cycles,  556.166 MOPS
vecTmult_Neon            :   2.67 cycles, avg   3.00 cycles,  832.577 MOPS
vecTmult_NeonPar2        :   2.59 cycles, avg   2.92 cycles,  855.339 MOPS
```

### Benchmark - Graviton4

Server aarch64.

```
matmult_ref              :  23.24 cycles, avg  23.68 cycles,  118.231 MOPS
matmult_novec            :  24.61 cycles, avg  26.60 cycles,  105.273 MOPS
matmult_Neon             :   6.84 cycles, avg   7.93 cycles,  353.232 MOPS
matmult_NeonPar2         :   6.84 cycles, avg   7.92 cycles,  353.559 MOPS
matmult_NeonPar4         :   6.84 cycles, avg   7.93 cycles,  353.223 MOPS
matmult_SveRows          :   7.52 cycles, avg  10.59 cycles,  264.391 MOPS
matmult_SveSingle        :   4.10 cycles, avg   5.15 cycles,  544.217 MOPS
vecmult_ref              :   4.70 cycles, avg   4.75 cycles,  589.779 MOPS
vecmult_novec            :   5.81 cycles, avg   5.89 cycles,  475.095 MOPS
vecmult_Neon             :   1.71 cycles, avg   1.96 cycles, 1426.544 MOPS
vecmult_NeonPar2         :   1.62 cycles, avg   1.95 cycles, 1433.017 MOPS
vecmult_Sve              :   0.26 cycles, avg   0.37 cycles, 7666.489 MOPS
vecTmult_ref             :   4.70 cycles, avg   4.81 cycles,  582.087 MOPS
vecTmult_Neon            :   2.31 cycles, avg   2.65 cycles, 1055.110 MOPS
vecTmult_NeonPar2        :   2.14 cycles, avg   2.58 cycles, 1087.370 MOPS
```

### Benchmark - Apple M1 Pro @ 3.228GHz

Apple M1 Pro laptop.

```
matmult_ref              :  12.61 cycles, avg  17.24 cycles,  187.191 MOPS
matmult_novec            :  24.43 cycles, avg  25.46 cycles,  126.750 MOPS
matmult_Neon             :   5.52 cycles, avg   6.12 cycles,  527.270 MOPS
matmult_NeonPar2         :   5.52 cycles, avg   6.16 cycles,  524.206 MOPS
matmult_NeonPar4         :   5.52 cycles, avg   6.10 cycles,  529.484 MOPS
vecmult_ref              :   3.35 cycles, avg   3.51 cycles,  918.997 MOPS
vecmult_novec            :   5.91 cycles, avg   6.22 cycles,  518.622 MOPS
vecmult_Neon             :   1.38 cycles, avg   1.52 cycles, 2120.846 MOPS
vecmult_NeonPar2         :   1.38 cycles, avg   1.46 cycles, 2214.777 MOPS
vecTmult_ref             :   7.29 cycles, avg   7.50 cycles,  430.458 MOPS
vecTmult_Neon            :   1.77 cycles, avg   1.91 cycles, 1690.740 MOPS
vecTmult_NeonPar2        :   1.87 cycles, avg   1.99 cycles, 1618.251 MOPS
```

### Build:

```
sudo apt -y update && sudo apt -y install cmake g++ git

git clone https://github.com/kvr000/zbynek-cxx-exp/
cd zbynek-cxx-exp/simd/matrix-multiplication

cmake .
make -j2
./target/bin/MatrixMultiplicationBenchmark
```


## License

The code is released under version 2.0 of the [Apache License][].

## Stay in Touch

Feel free to contact me at Zbynek Vyskovsky - kvr000@gmail.com or http://github.com/kvr000 and https://www.linkedin.com/in/zbynek-vyskovsky/ .

[Apache License]: http://www.apache.org/licenses/LICENSE-2.0
