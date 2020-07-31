cmake_minimum_required(VERSION 3.16)
project(BIE_PJP_MilaLanguageCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

add_definitions("-fexceptions")

execute_process(COMMAND llvm-config --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE CMAKE_CXX_FLAGS)
string(STRIP ${CMAKE_CXX_FLAGS} CMAKE_CXX_FLAGS)

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
        include/Expression.h
        source/CodeGenerator.cpp
        include/CodeGenerator.h
        include/TextPosition.h source/externs.cpp include/Operators.h)

#llvm_map_components_to_libnames(llvm_libs support core irreader executionEngine)

target_link_libraries(BIE_PJP_MilaLanguageCompiler ${llvm_libs})



