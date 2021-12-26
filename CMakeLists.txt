cmake_minimum_required(VERSION 3.5.0)

project(drag)


add_library(drag INTERFACE)

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
    find_path(DIRENT_INCLUDE_DIRS "dirent.h")
    target_include_directories(drag INTERFACE ${DIRENT_INCLUDE_DIRS})
endif()

target_include_directories(drag INTERFACE include/)
target_compile_features(drag INTERFACE cxx_std_17)


if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_subdirectory(example/)
    add_subdirectory(benchmarks/)
    add_subdirectory(test/)
endif()
