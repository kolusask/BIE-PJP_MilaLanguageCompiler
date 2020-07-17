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
    static TokenType check_character(const char ch);
    static TokenType check_keyword(const std::string& word);
    static TokenType check_operator(const std::string& op);
    static bool is_datatype(TokenType dt);
    static bool is_delimiter(const char del);
};

#endif //MILALANGUAGECOMPILER_SYNTAX_H
