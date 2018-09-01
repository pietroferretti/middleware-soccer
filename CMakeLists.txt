cmake_minimum_required(VERSION 3.8)
project(middleware-soccer)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_COMPILER "mpicc")

#set(CMAKE_C_FLAGS "-O3 -g3")

include_directories(include/)
set(SOURCE_FILES ./source/main.c ./source/parser.c ./source/possession.c ./source/output.c)
add_executable(soccer ${SOURCE_FILES})

add_executable(sequential_soccer sequential.c)
#target_link_libraries(m)
