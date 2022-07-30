cmake_minimum_required(VERSION 3.14)
project(clib-exp)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/target/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG HEAD
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  clib-exp
  src/main/cxx/sprintf-test.cxx
)
target_link_libraries(
  clib-exp
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(clib-exp)
