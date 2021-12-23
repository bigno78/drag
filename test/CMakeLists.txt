add_library(test-utils utils/bruteforce.cpp)
target_link_libraries(test-utils PUBLIC drag)

add_executable(opt test-optimality.cpp)
target_link_libraries(opt test-utils)

set(TEST_SOURCES test-cycle.cpp test-graph.cpp test-layering.cpp test-subgraph.cpp)

add_executable(tests test-main.cpp ${TEST_SOURCES})
target_link_libraries(tests test-utils)
