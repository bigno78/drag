add_library(test-utils utils/bruteforce.cpp)
target_link_libraries(test-utils PUBLIC drag)

add_executable(playground my-main.cpp)
target_link_libraries(playground test-utils)

add_executable(rnd test-randomized.cpp)
target_link_libraries(rnd test-utils)
