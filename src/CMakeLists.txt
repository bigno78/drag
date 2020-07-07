cmake_minimum_required(VERSION 3.5.0)

project(bakalarka)

#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17")

add_library(debug_options INTERFACE)
target_compile_options(debug_options INTERFACE -g -std=c++17 -fsanitize=address -fsanitize=undefined)
target_link_libraries(debug_options INTERFACE -fsanitize=address -fsanitize=undefined)

#target_compile_options(debug_options INTERFACE -g -std=c++17)
#target_link_libraries(debug_options INTERFACE)

add_library(bench_options INTERFACE)
target_compile_options(bench_options INTERFACE -O2 -std=c++17)

add_executable(tests test/test-main.cpp test/test-layering.cpp test/test-graph.cpp)
target_link_libraries(tests PRIVATE debug_options)

add_executable(dot dot.cpp)
target_link_libraries(dot PRIVATE debug_options)

add_executable(example example/example.cpp)
target_link_libraries(example PRIVATE debug_options)

add_executable(cycl ../stats/cyclify.cpp)
target_link_libraries(cycl PRIVATE debug_options)
target_include_directories(cycl PRIVATE ./)

add_executable(stats ../stats/stats.cpp)
target_link_libraries(stats PRIVATE debug_options)
target_include_directories(stats PRIVATE ./)

add_executable(bench benchmarks/bench.cpp)
target_link_libraries(bench PRIVATE bench_options)

add_executable(draw benchmarks/draw.cpp)
target_link_libraries(draw PRIVATE debug_options)

