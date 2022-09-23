cmake_minimum_required(VERSION 3.13)  # CMake version check
project(AGIZero)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

set(OUT AGIZero)

if (WIN32)
    add_compile_definitions(WINDOWS)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-march=native)
    add_compile_options(-O3)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/O2)
endif ()

if (UNIX)
    add_link_options(-pthread)
endif()



# Add main.cpp file of project root directory as source file
set(SRCS main.cpp
    alphabeta.cpp benchmark.cpp board.cpp eval.cpp material.cpp 
    movegen.cpp options.cpp pawneval.cpp pieces.cpp position.cpp
    printer.cpp search.cpp table.cpp threads.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(${OUT} ${SRCS})