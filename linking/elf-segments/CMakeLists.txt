cmake_minimum_required(VERSION 2.4)

project(ElfSegments)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/target/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(runner)
add_subdirectory(lib1)
