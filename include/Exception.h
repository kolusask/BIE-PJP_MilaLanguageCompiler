//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_EXCEPTION_H
#define MILALANGUAGECOMPILER_EXCEPTION_H

#include <string>

#include "Lexer.h"

class Exception {
public:
    Exception(const TextPosition& pos, std::string message) :
        m_message(std::string("Line ") + std::to_string(pos.line) + ", column " + std::to_string(pos.column) + ":\n"
        + message) {}
    std::string message() const { return m_message; }

protected:
    const std::string m_message;
};

class InvalidSymbolException : public Exception {
public:
    InvalidSymbolException(const TextPosition& pos, const char inv) :
        Exception(pos, "Invalid symbol: '" + (inv != '\n' ? std::string(1, inv) : std::string("\\n")) + '\'') {}
};

class UnexpectedTokenException : public Exception {
public:
    UnexpectedTokenException(const TextPosition& pos, const std::string& token) :
        Exception(pos, std::string("Unexpected token: '") + token + '\'') {}
};

class ExpectedDifferentException : public Exception {
public:
    ExpectedDifferentException(const TextPosition& pos, const std::string& token) :
        Exception(pos, std::string("Expected '") + token + '\'') {}
};

#endif //MILALANGUAGECOMPILER_EXCEPTION_H
