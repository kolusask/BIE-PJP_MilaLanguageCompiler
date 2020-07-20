cmake_minimum_required(VERSION 3.16)
project(BIE_PJP_MilaLanguageCompiler)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


add_executable(BIE_PJP_MilaLanguageCompiler
        main.cpp
        include/Token.h
        include/Syntax.h
        include/Exception.h
        source/Lexer.cpp
        include/Lexer.h
        source/Parser.cpp
        include/Parser.h
        source/Syntax.cpp
        source/Token.cpp
        source/Expression.cpp
        include/Expression.h source/CodeGenerator.cpp include/CodeGenerator.h include/TextPosition.h)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(BIE_PJP_MilaLanguageCompiler ${llvm_libs})
