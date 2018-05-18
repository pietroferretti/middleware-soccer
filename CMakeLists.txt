cmake_minimum_required(VERSION 3.8)
project(middleware-soccer)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_COMPILER "/home/nicole/mpich-install/bin/mpicc")

#set(CMAKE_C_COMPILER "/home/.../mpich-install/bin/mpicc") tutta tua :)

set(SOURCE_FILES main.c)
add_executable(soccer ${SOURCE_FILES})

#target_link_libraries(m)