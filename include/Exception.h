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
    InvalidSymbolException(const size_t line, const size_t column, const char invalid) :
        Exception(std::string("Line ") + std::to_string(line) + ", column " + std::to_string(column)
        + ":\nInvalid symbol: '" + (invalid != '\n' ? std::string(1, invalid) : std::string("\\n")) + '\'') {}
};

#endif //MILALANGUAGECOMPILER_EXCEPTION_H
