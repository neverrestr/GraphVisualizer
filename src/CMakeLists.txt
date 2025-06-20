cmake_minimum_required(VERSION 3.19)

project(graph-serializer VERSION 0.0 LANGUAGES CXX)

# Set sources and headers (headers optional - for some IDEs)
set(SOURCES main.cpp)
set(HEADERS )

set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(common)
add_subdirectory(parser)
add_subdirectory(lexer)
add_subdirectory(tests)
add_subdirectory(algorithms)
add_subdirectory(utils) #new

add_executable(graph-serializer ${SOURCES} ${HEADERS})

target_link_libraries(graph-serializer PRIVATE lexer)
target_link_libraries(graph-serializer PRIVATE parser)
target_link_libraries(graph-serializer PRIVATE common)
target_link_libraries(graph-serializer PRIVATE tiny-fsm)
target_link_libraries(graph-serializer PRIVATE algorithms)
target_link_libraries(graph-serializer PRIVATE utils) #new