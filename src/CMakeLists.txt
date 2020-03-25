cmake_minimum_required(VERSION 3.0.0)

project(bakalarka VERSION 0.1.0)

include(CTest)
enable_testing()
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -std=c++17")

add_executable(bakalarka dot.cpp)

add_executable(tests test/test-main.cpp test/test-subgraph.cpp test/test-cycle.cpp test/test-layering.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
