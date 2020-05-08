//
// Created by askar on 08/05/2020.
//

#include "../include/Token.h"


const std::map<TokenType, std::string> SimpleToken::s_TokStrings = {{TOK_ASSIGN, "="},
                                                                    {TOK_BEGIN, "begin"},
                                                                    {TOK_CLOSE_BRACKET, ")"},
                                                                    {TOK_CONST, "const"},
                                                                    {TOK_DOT, "."},
                                                                    {TOK_END, "end"},
                                                                    {TOK_OPEN_BRACKET, ")"},
                                                                    {TOK_PROGRAM, "program"},
                                                                    {TOK_SEMICOLON, ";"}};
