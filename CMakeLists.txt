cmake_minimum_required(VERSION 3.5.0)

project(drag)


add_library(drag INTERFACE)

target_include_directories(drag INTERFACE include/)
target_compile_features(drag INTERFACE cxx_std_17)


if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_subdirectory(example/)
    add_subdirectory(benchmarks/)
    add_subdirectory(test/)
endif()
