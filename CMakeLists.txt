# Force using Clang
if (NOT CMAKE_CXX_COMPILER MATCHES ".*clang\\+\\+$")
    message(FATAL_ERROR "Please use Clang: run cmake with -DCMAKE_CXX_COMPILER=clang++")
endif()

cmake_minimum_required(VERSION 3.16)
project(Delirium LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add source files
set(SOURCES
    src/main.cpp
    src/disassembler.cpp
    src/bytecode.cpp 
)

set(HEADERS
    include/disassembler.hpp
    include/bytecode.hpp
    include/opcode.hpp
)

# Define executable
add_executable(delirium ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(delirium PRIVATE include)

# Enable sanitizers in debug mode (optional, useful for debugging)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling sanitizers for Debug mode")
    target_compile_options(delirium PRIVATE -fsanitize=address,undefined)
    target_link_options(delirium PRIVATE -fsanitize=address,undefined)
endif()

# Add tests (optional)
# enable_testing()
# add_subdirectory(tests)

# Installation (optional)
# install(TARGETS delirium DESTINATION bin)
