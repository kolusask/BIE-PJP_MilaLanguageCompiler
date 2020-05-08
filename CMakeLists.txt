cmake_minimum_required(VERSION 3.16)
project(BIE_PJP_MilaLanguageCompiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(BIE_PJP_MilaLanguageCompiler main.cpp include/Token.h include/Syntax.h include/Exception.h source/Lexer.cpp include/Lexer.h Parser.cpp Parser.h source/Syntax.cpp)