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
    TOK_AND,
    TOK_ASSIGN,
    TOK_BEGIN,
    TOK_BREAK,
    TOK_CLOSE_BRACKET,
    TOK_COMMA,
    TOK_COLON,
    TOK_CONST,
    TOK_DIV,
    TOK_DIVIDE,
    TOK_DO,
    TOK_DOT,
    TOK_DOUBLE,
    TOK_DOWNTO,
    TOK_END,
    TOK_ELSE,
    TOK_EOF,
    TOK_EQUAL,
    TOK_EXIT,
    TOK_FOR,
    TOK_FORWARD,
    TOK_FUNCTION,
    TOK_IDENTIFIER,
    TOK_IF,
    TOK_INTEGER,
    TOK_LESS,
    TOK_LESS_OR_EQUAL,
    TOK_MINUS,
    TOK_MOD,
    TOK_GREATER,
    TOK_GREATER_OR_EQUAL,
    TOK_MULTIPLY,
    TOK_NOT_EQUAL,
    TOK_OPEN_BRACKET,
    TOK_OR,
    TOK_PLUS,
    TOK_PROCEDURE,
    TOK_PROGRAM,
    TOK_SEMICOLON,
    TOK_STRING,
    TOK_THEN,
    TOK_TO,
    TOK_VAR,
    TOK_VOID,
    TOK_WHILE
};

// Base class
class Token : std::enable_shared_from_this<Token> {
public:
    virtual TokenType type() const = 0;
    virtual std::string to_string() const = 0;
    virtual int op_precedence() const { return -1; }
};

// 'begin', 'const', 'end', 'program', '=', ';', '.', '(', ')'
class SimpleToken: public Token {
public:
    SimpleToken(const TokenType type) : m_type(type) {}
    virtual TokenType type() const override { return m_type; }
    std::string to_string() const override {
        const static std::map<TokenType, std::string> tokStrings = {
                                                                    {TOK_EQUAL, "="},
                                                                    {TOK_BEGIN, "begin"},
                                                                    {TOK_CLOSE_BRACKET, ")"},
                                                                    {TOK_CONST, "const"},
                                                                    {TOK_DOT, "."},
                                                                    {TOK_END, "end"},
                                                                    {TOK_OPEN_BRACKET, ")"},
                                                                    {TOK_PROGRAM, "program"},
                                                                    {TOK_SEMICOLON, ";"},
                                                                    {TOK_COMMA, ","},
                                                                    {TOK_FORWARD, "forward"}};
        auto it = tokStrings.find(m_type);
        if (it == tokStrings.end())
            return "<?>";
        return it->second;
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
    IntegerToken(int value) : m_value(value) {}
    TokenType type() const override { return TOK_INTEGER; }
    int value() const { return m_value; }
    std::string to_string() const override { return std::to_string(m_value); }

private:
    const int m_value;
};

class DoubleToken : public Token {
public:
    DoubleToken(double value) : m_value(value) {}
    TokenType type() const override { return TOK_DOUBLE; }
    double value() const { return m_value; }
    std::string to_string() const override { return std::to_string(m_value); }

private:
    const double m_value;
};

class OperatorToken : public Token {
public:
    OperatorToken(const TokenType type) : m_type(type) {}
    int op_precedence() const override;
    TokenType type() const override { return m_type; }
    std::string to_string() const override;

private:
    const TokenType m_type;
};

class StringToken : public Token {
public:
    StringToken(std::string str) : m_string(std::move(str)) {}
    TokenType type() const override { return TOK_STRING; }
    std::string to_string() const override { return '"' + m_string + '"'; }
    std::string string() const { return m_string; }

private:
    const std::string m_string;
};

#endif //MILALANGUAGECOMPILER_TOKENS_H