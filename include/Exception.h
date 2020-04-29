//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_EXCEPTION_H
#define MILALANGUAGECOMPILER_EXCEPTION_H

#include <string>

class Exception {
public:
    Exception(const std::string message) : m_Message(message) {}
    std::string message() const { return m_Message; }

protected:
    const std::string m_Message;
};

class InvalidSymbolException : public Exception {
public:
    InvalidSymbolException(const TextPosition& pos, char inv) :
        Exception(std::string("Line ") + std::to_string(pos.line) + ", column " + std::to_string(pos.column)
        + ":\nInvalid symbol: '" + (inv != '\n' ? std::string(1, inv) : std::string("\\n")) + '\'') {}
};

#endif //MILALANGUAGECOMPILER_EXCEPTION_H
