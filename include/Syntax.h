//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_SYNTAX_H
#define MILALANGUAGECOMPILER_SYNTAX_H

#include "Token.h"

#include <map>
#include <set>


class Syntax {
public:
    static TokenType check_character(const char op);
    static TokenType check_keyword(const std::string& word);
    static bool is_delimiter(const char del);
private:
    static const std::set<char> s_delimiters;
    static const std::map<std::string, TokenType> s_keyWords;
    static const std::map<char, TokenType> s_characters;
    static const std::map<TokenType, std::string> s_simpleTokens;
};

#endif //MILALANGUAGECOMPILER_SYNTAX_H
