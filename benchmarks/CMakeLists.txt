
add_executable(cycl cyclify.cpp)
target_link_libraries(cycl drag)

add_executable(stats stats.cpp)
target_link_libraries(stats drag)

add_executable(cross cross.cpp)
target_link_libraries(cross drag)
