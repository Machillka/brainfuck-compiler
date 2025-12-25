cmake_minimum_required(VERSION 3.10)
project(bf_compiler_cpp LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



include_directories(include)

add_executable(bf_compiler
    src/lexer.cpp
    src/parser.cpp
    # src/ast.cpp
    src/ir.cpp
    src/optimizer.cpp
    src/codegen_x86.cpp
    src/runtime.cpp
    src/main.cpp
)
