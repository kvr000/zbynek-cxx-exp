cmake_minimum_required(VERSION 2.4)

project(Instructions)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/target/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(Instructions src/main/cxx/Main.cxx)
add_executable(Instructions src/main/cxx/Logical.cxx)
