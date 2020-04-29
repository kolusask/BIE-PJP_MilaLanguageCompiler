//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_SYNTAX_H
#define MILALANGUAGECOMPILER_SYNTAX_H

#include "../include/Token.h"

#include <map>
#include <set>


class Syntax {
public:
    static TokenType check_character(const char op);
    static TokenType check_keyword(const std::string& word);
    static bool is_delimiter(const char del);
private:
    static const std::set<char> s_Delimiters;
    static const std::map<std::string, TokenType> s_KeyWords;
    static const std::map<char, TokenType> s_Characters;
};

const std::set<char> Syntax::s_Delimiters = { ' ', '\n', '\t' };

const std::map<std::string, TokenType> Syntax::s_KeyWords = {{"begin", TOK_BEGIN},
                                                            {"const", TOK_CONST},
                                                            {"end", TOK_END},
                                                            {"program", TOK_PROGRAM} };

const std::map<char, TokenType> Syntax::s_Characters = {{'=', TOK_ASSIGN},
                                                        {'.', TOK_DOT},
                                                        {';', TOK_SEMICOLON},
                                                        {'(', TOK_OPEN_BRACKET},
                                                        {')', TOK_CLOSE_BRACKET}};

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

#endif //MILALANGUAGECOMPILER_SYNTAX_H
