//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_EXCEPTION_H
#define MILALANGUAGECOMPILER_EXCEPTION_H

#include <string>

class Exception {
public:
    Exception(const TextPosition& pos, std::string message) :
        m_Message(std::string("Line ") + std::to_string(pos.line) + ", column " + std::to_string(pos.column) + ":\n"
        + message) {}
    std::string message() const { return m_Message; }

protected:
    const std::string m_Message;
};

class InvalidSymbolException : public Exception {
public:
    InvalidSymbolException(const TextPosition& pos, const char inv) :
        Exception(pos, "Invalid symbol: '" + (inv != '\n' ? std::string(1, inv) : std::string("\\n")) + '\'') {}
};

class UnexpectedTokenException : public Exception {
    UnexpectedTokenException(const TextPosition& pos, const std::string& token) :
        Exception(pos, std::string("Unexpected token: '") + token + '\'') {}
};

#endif //MILALANGUAGECOMPILER_EXCEPTION_H
