//
// Created by askar on 08/05/2020.
//

#include "../include/Syntax.h"

#include <set>


const std::list<Syntax::Operator> Syntax::s_operators = {{TOK_PLUS, "+", false},
                                                         {TOK_MINUS, "-", false},
                                                         {TOK_MULTIPLY, "*", false},
                                                         {TOK_EQUAL, "=", false},
                                                         {TOK_MOD, "mod", false},
                                                         {TOK_MORE, ">", true},
                                                         {TOK_LESS, "<", true},
                                                         {TOK_ASSIGN, ":=", false}};

TokenType Syntax::check_character(const char ch) {
    static const std::map<char, TokenType> characters = {{'=', TOK_EQUAL},
                                                         {'.', TOK_DOT},
                                                         {';', TOK_SEMICOLON},
                                                         {'(', TOK_OPEN_BRACKET},
                                                         {')', TOK_CLOSE_BRACKET},
                                                         {',', TOK_COMMA},
                                                         {':', TOK_COLON}};
    auto it = characters.find(ch);
    if (it != characters.end())
        return it->second;
    return TOK_INVALID;
}

TokenType Syntax::check_keyword(const std::string &word) {
    static const std::map<std::string, TokenType> keyWords = {{"begin", TOK_BEGIN},
                                                              {"break", TOK_BREAK},
                                                              {"const", TOK_CONST},
                                                              {"end", TOK_END},
                                                              {"program", TOK_PROGRAM},
                                                              {"var", TOK_VAR},
                                                              {"integer", TOK_INTEGER},
                                                              {"function", TOK_FUNCTION},
                                                              {"if", TOK_IF},
                                                              {"then", TOK_THEN},
                                                              {"else", TOK_ELSE},
                                                              {"while", TOK_WHILE},
                                                              {"do", TOK_DO},
                                                              {"for", TOK_FOR},
                                                              {"to", TOK_TO},
                                                              {"downto", TOK_DOWNTO}};
    auto it = keyWords.find(word);
    if (it != keyWords.end())
        return it->second;
    return TOK_INVALID;
}

bool Syntax::is_delimiter(const char del) {
    static const std::set<char> delimiters = { ' ',
                                               '\n',
                                               '\t' };
    return delimiters.count(del);
}

TokenType Syntax::check_operator(const std::string &op) {
    static std::map<std::string, TokenType> opMap;
    if (opMap.empty())
        for (const Operator& op : s_operators)
            opMap[op.name] = op.type;
    auto it = opMap.find(op);
    if (it != opMap.end())
        return it->second;
    return TOK_INVALID;
}

bool Syntax::is_datatype(TokenType dt) {
    static const std::set<TokenType> dataTypes = {TOK_INTEGER};
    return dataTypes.count(dt);
}

bool Syntax::is_bool_operator(TokenType op) {
    static std::set<TokenType> bool_ops;
    if (bool_ops.empty())
        for (const auto& op : s_operators)
            if (op.is_boolean)
                bool_ops.insert(op.type);
    return bool_ops.count(op);
}
