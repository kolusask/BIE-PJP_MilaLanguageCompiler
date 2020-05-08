//
// Created by askar on 08/05/2020.
//

#include "../include/Syntax.h"

const std::set<char> Syntax::s_Delimiters = { ' ', '\n', '\t' };

const std::map<std::string, TokenType> Syntax::s_KeyWords = {{"begin", TOK_BEGIN},
                                                             {"const", TOK_CONST},
                                                             {"end", TOK_END},
                                                             {"program", TOK_PROGRAM}};

const std::map<char, TokenType> Syntax::s_Characters = {{'=', TOK_ASSIGN},
                                                        {'.', TOK_DOT},
                                                        {';', TOK_SEMICOLON},
                                                        {'(', TOK_OPEN_BRACKET},
                                                        {')', TOK_CLOSE_BRACKET}};

const std::map<TokenType, std::string> Syntax::s_SimpleTokens = {{TOK_ASSIGN, "="},
                                                                 {TOK_BEGIN, "begin"},
                                                                 {TOK_CLOSE_BRACKET, ")"},
                                                                 {TOK_CONST, "const"},
                                                                 {TOK_DOT, "."},
                                                                 {TOK_END, "end"},
                                                                 {TOK_OPEN_BRACKET, ")"},
                                                                 {TOK_PROGRAM, "program"},
                                                                 {TOK_SEMICOLON, ";"}};

std::string Syntax::simple_token(const TokenType type) {
    return s_SimpleTokens.at(type);
}

TokenType Syntax::check_character(const char op) {
    if (s_Characters.count(op))
        return s_Characters.at(op);
    return TOK_INVALID;
}

TokenType Syntax::check_keyword(const std::string &word) {
    if (s_KeyWords.count(word))
        return s_KeyWords.at(word);
    return TOK_INVALID;
}

bool Syntax::is_delimiter(const char del) {
    return s_Delimiters.count(del);
}
