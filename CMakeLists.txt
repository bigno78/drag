cmake_minimum_required(VERSION 3.5.0)

project(bakalarka)

#file(COPY data/ DESTINATION data/)

# helper targets for different compile and link options
add_library(options INTERFACE)
target_include_directories(options INTERFACE	 include/)

add_library(debug_options INTERFACE)
target_compile_options(debug_options INTERFACE -g -std=c++17 -fsanitize=address -fsanitize=undefined)
target_link_libraries(debug_options INTERFACE options -fsanitize=address -fsanitize=undefined)

add_library(bench_options INTERFACE)
target_compile_options(bench_options INTERFACE -O2 -std=c++17)
target_link_libraries(bench_options INTERFACE options)


# actual targets
add_executable(draw example/draw.cpp)
target_link_libraries(draw PRIVATE debug_options)

add_executable(cycl stats/cyclify.cpp)
target_link_libraries(cycl PRIVATE debug_options)
target_include_directories(cycl PRIVATE example/)

add_executable(stats stats/stats.cpp)
target_link_libraries(stats PRIVATE debug_options)
target_include_directories(stats PRIVATE example/)
