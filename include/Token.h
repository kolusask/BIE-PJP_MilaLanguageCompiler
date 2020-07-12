//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_TOKENS_H
#define MILALANGUAGECOMPILER_TOKENS_H

#include <map>
#include <memory>
#include <string>

enum TokenType {
    TOK_INVALID = 0,
    TOK_BEGIN,
    TOK_CLOSE_BRACKET,
    TOK_CONST,
    TOK_DOT,
    TOK_END,
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_INIT,
    TOK_INTEGER,
    TOK_OPEN_BRACKET,
    TOK_PROGRAM,
    TOK_SEMICOLON
};

// Base class
class Token : std::enable_shared_from_this<Token> {
public:
    virtual TokenType type() const = 0;
    virtual std::string to_string() const = 0;

    template<typename T>
    std::shared_ptr<T> as() {
        return std::static_pointer_cast<T>(shared_from_this());
    }
};

// 'begin', 'const', 'end', 'program', '=', ';', '.', '(', ')'
class SimpleToken: public Token {
public:
    SimpleToken(const TokenType type) : m_Type(type) {}
    virtual TokenType type() const override { return m_Type; }
    std::string to_string() const override { return s_TokStrings.at(m_Type); }

private:
    static const std::map<TokenType, std::string> s_TokStrings;
    const TokenType m_Type;
};

class IdentifierToken : public Token {
public:
    IdentifierToken(const std::string& name) : m_Name(name) {}
    TokenType type() const override { return TOK_IDENTIFIER; }
    std::string name() const { return m_Name; }
    std::string to_string() const override { return m_Name; }

private:
    const std::string m_Name;
};


class TokInteger : public Token {
public:
    TokInteger(const int value) : m_Value(value) {}
    TokenType type() const override { return TOK_INTEGER; }
    int value() const { return m_Value; }
    std::string to_string() const override { return std::to_string(m_Value); }

private:
    const int m_Value;
};

#endif //MILALANGUAGECOMPILER_TOKENS_H