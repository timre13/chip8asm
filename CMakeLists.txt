cmake_minimum_required(VERSION 3.10)

project(asm VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -g3")

add_definitions(-DNOT_CLANGD)

add_executable(chip8asm
    src/main.cpp
    src/Logger.cpp
    src/InputFile.cpp
    src/parser.cpp
    src/binary_generator.cpp
    src/arguments.cpp
)

