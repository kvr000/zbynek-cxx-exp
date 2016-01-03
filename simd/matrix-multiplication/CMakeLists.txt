cmake_minimum_required(VERSION 2.4)

project(MatrixMultiplicationBenchmark)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/target/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3 -march=native")

set(SOURCES
	src/main/cxx/MatrixMultiplicationBenchmark.cxx
	src/main/cxx/MatrixMultiplicationNoVectorize.cxx
)
# this works for host processor but let not assume someone would build cross-platform
if ((CMAKE_SYSTEM_PROCESSOR STREQUAL "i386") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64"))
	list(APPEND SOURCES src/main/cxx/MatrixMultiplicationFpu87.cxx)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set_property(SOURCE src/main/cxx/MatrixMultiplicationNoVectorize.cxx PROPERTY COMPILE_FLAGS "-fno-vectorize -fno-tree-vectorize -fno-slp-vectorize")
	set_property(SOURCE src/main/cxx/MatrixMultiplicationFpu87.cxx PROPERTY COMPILE_FLAGS "-fno-vectorize -fno-tree-vectorize -fno-slp-vectorize -mno-sse -DNO_VECTORIZE")
else()
	set_property(SOURCE src/main/cxx/MatrixMultiplicationNoVectorize.cxx PROPERTY COMPILE_FLAGS "-fno-tree-vectorize -fno-tree-slp-vectorize")
	set_property(SOURCE src/main/cxx/MatrixMultiplicationFpu87.cxx PROPERTY COMPILE_FLAGS "-fno-tree-vectorize -fno-tree-slp-vectorize -mno-sse -DNO_VECTORIZE")
endif()

add_executable(MatrixMultiplicationBenchmark
	${SOURCES}
)
