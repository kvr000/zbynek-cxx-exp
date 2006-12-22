set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} src/main/linker/linker.ld")
add_executable(runner src/main/cxx/main.cxx src/main/cxx/runner_part1.cxx)
target_link_libraries(runner LINK_PUBLIC lib1)
set_target_properties(runner PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/runner/src/main/linker/linker.ld)
