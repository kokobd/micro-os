cmake_minimum_required(VERSION 3.9)

project(micro-os VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

enable_testing()

add_subdirectory(src/core)
add_subdirectory(src/kernel)
