//
// Created by askar on 31/07/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_OPERATORS_H
#define BIE_PJP_MILALANGUAGECOMPILER_OPERATORS_H

#include "Token.h"

#include <set>


struct Operator {
    const TokenType type;
    const std::string name;
    const int precedence;
    const bool is_boolean;
    bool operator<(const Operator& other) const {
        return this->type < other.type;
    }
};

const std::set<Operator> g_operators = {{TOK_ASSIGN, ":=", 5, false},
                                       {TOK_DIVIDE, "/", 40, false},
                                       {TOK_EQUAL, "=", 10, true},
                                       {TOK_LESS, "<", 10, true},
                                       {TOK_LESS_OR_EQUAL, "<=", 10, true},
                                       {TOK_MINUS, "-", 20, false},
                                       {TOK_MOD, "mod", 40, false},
                                       {TOK_GREATER, ">", 10, true},
                                       {TOK_GREATER_OR_EQUAL, ">=", 10, true},
                                       {TOK_MULTIPLY, "*", 40, false},
                                       {TOK_NOT_EQUAL, "<>", 10, true},
                                       {TOK_PLUS, "+", 20, false},
                                       {TOK_AND, "and", 10, true},
                                       {TOK_OR, "or", 10, true},
                                       {TOK_DIV, "div", 40, false}};

#endif //BIE_PJP_MILALANGUAGECOMPILER_OPERATORS_H
