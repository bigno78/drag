cmake_minimum_required(VERSION 3.0.0)

project(bakalarka)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -std=c++17")

add_executable(dot dot.cpp)
add_executable(example example.cpp)

file(COPY data/ DESTINATION data/)

#add_executable(tests test/test-main.cpp test/test-subgraph.cpp test/test-cycle.cpp test/test-layering.cpp)
