add_library(test-utils utils/bruteforce.cpp)
target_link_libraries(test-utils PUBLIC drag)

add_executable(playground my-main.cpp)
target_link_libraries(playground test-utils)

add_executable(opt test-optimality.cpp)
target_link_libraries(opt test-utils)
