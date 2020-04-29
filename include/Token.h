//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_TOKENS_H
#define MILALANGUAGECOMPILER_TOKENS_H

#include <string>

enum TokenType {
    TOK_INVALID = 0,
    TOK_ASSIGN,
    TOK_BEGIN,
    TOK_CLOSE_BRACKET,
    TOK_CONST,
    TOK_DOT,
    TOK_END,
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_INTEGER,
    TOK_OPEN_BRACKET,
    TOK_PROGRAM,
    TOK_SEMICOLON
};

// Base class
class Token {
public:
    virtual TokenType type() const = 0;
};

// 'begin', 'const', 'end', 'program', '=', ';', '.', '(', ')'
class SimpleToken: public Token {
public:
    SimpleToken(const TokenType type) : m_Type(type) {}
    virtual TokenType type() const override { return m_Type; }

private:
    const TokenType m_Type;
};

class IdentifierToken : public Token {
public:
    IdentifierToken(const std::string& name) : m_Name(name) {}
    TokenType type() const override { return TOK_IDENTIFIER; }
    std::string name() const { return m_Name; }

private:
    const std::string m_Name;
};

class TokInteger : public Token {
public:
    TokInteger(const int value) : m_Value(value) {}
    TokenType type() const override { return TOK_INTEGER; }
    int value() const { return m_Value; }

private:
    const int m_Value;
};

#endif //MILALANGUAGECOMPILER_TOKENS_H
