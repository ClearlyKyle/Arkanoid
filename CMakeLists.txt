cmake_minimum_required(VERSION 3.10)

project(Arkanoid)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 CONFIG REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} main.c)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2)