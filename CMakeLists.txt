cmake_minimum_required(VERSION 3.16)
project(matrix2code VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

add_executable(compiler
    src/main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/AST.cpp
    src/SymbolTable.cpp
    src/SemanticAnalyzer.cpp
    src/IR.cpp
    src/IRGenerator.cpp
    src/Optimizer.cpp
    src/CodeGenerator.cpp
)

target_include_directories(compiler PRIVATE include)

add_executable(compiler_test
    tests/test_main.cpp
    tests/test_lexer.cpp
    tests/test_parser.cpp
    tests/test_semantic.cpp
    tests/test_optimizer.cpp
    tests/test_integration.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/AST.cpp
    src/SymbolTable.cpp
    src/SemanticAnalyzer.cpp
    src/IR.cpp
    src/IRGenerator.cpp
    src/Optimizer.cpp
    src/CodeGenerator.cpp
)

target_include_directories(compiler_test PRIVATE include tests)
