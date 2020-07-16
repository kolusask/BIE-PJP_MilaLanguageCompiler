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
    TOK_COMMA,
    TOK_CONST,
    TOK_DOT,
    TOK_END,
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_EQUAL,
    TOK_INTEGER,
    TOK_LESS,
    TOK_MINUS,
    TOK_MORE,
    TOK_MULTIPLY,
    TOK_OPEN_BRACKET,
    TOK_PLUS,
    TOK_PROGRAM,
    TOK_SEMICOLON
};

// Base class
class Token : std::enable_shared_from_this<Token> {
public:
    virtual TokenType type() const = 0;
    virtual std::string to_string() const = 0;
    virtual int op_precedence() const { return -1; }

    template<typename T>
    std::shared_ptr<T> as() {
        return std::static_pointer_cast<T>(shared_from_this());
    }
};

// 'begin', 'const', 'end', 'program', '=', ';', '.', '(', ')'
class SimpleToken: public Token {
public:
    SimpleToken(const TokenType type) : m_type(type) {}
    virtual TokenType type() const override { return m_type; }
    std::string to_string() const override {
        const static std::map<TokenType, std::string> tokStrings = {{TOK_EQUAL, "="},
                                                                    {TOK_BEGIN, "begin"},
                                                                    {TOK_CLOSE_BRACKET, ")"},
                                                                    {TOK_CONST, "const"},
                                                                    {TOK_DOT, "."},
                                                                    {TOK_END, "end"},
                                                                    {TOK_OPEN_BRACKET, ")"},
                                                                    {TOK_PROGRAM, "program"},
                                                                    {TOK_SEMICOLON, ";"},
                                                                    {TOK_COMMA, ","}};
        return tokStrings.at(m_type);
    }

private:
    const TokenType m_type;
};

class IdentifierToken : public Token {
public:
    IdentifierToken(const std::string& name) : m_name(name) {}
    TokenType type() const override { return TOK_IDENTIFIER; }
    std::string name() const { return m_name; }
    std::string to_string() const override { return m_name; }

private:
    const std::string m_name;
};


class IntegerToken : public Token {
public:
    IntegerToken(const int value) : m_value(value) {}
    TokenType type() const override { return TOK_INTEGER; }
    int value() const { return m_value; }
    std::string to_string() const override { return std::to_string(m_value); }

private:
    const int m_value;
};

class OperatorToken : public Token {
public:
    OperatorToken(const TokenType type) : m_type(type) {}
    int op_precedence() const override {
        static const std::map<TokenType, int> prec_map = {{TOK_PLUS, 20},
                                                          {TOK_MINUS, 20},
                                                          {TOK_MULTIPLY, 40}};
        return prec_map.at(m_type);
    }
    TokenType type() const override { return m_type; }
    std::string to_string() const override {
        const static std::map<TokenType, const char*> opStrings = {{TOK_PLUS, "+"},
                                                                   {TOK_MINUS, "-"},
                                                                   {TOK_MULTIPLY, "*"}};
        return opStrings.at(m_type);
    }

private:
    const TokenType m_type;
};

#endif //MILALANGUAGECOMPILER_TOKENS_H