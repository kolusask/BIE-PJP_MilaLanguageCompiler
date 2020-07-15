//
// Created by askar on 08/05/2020.
//

#include "../include/Syntax.h"

const std::set<char> Syntax::s_delimiters = { ' ', '\n', '\t' };

const std::map<std::string, TokenType> Syntax::s_keyWords = {{"begin", TOK_BEGIN},
                                                             {"const", TOK_CONST},
                                                             {"end", TOK_END},
                                                             {"program", TOK_PROGRAM}
                                                            };

const std::map<char, TokenType> Syntax::s_characters = {{'=', TOK_INIT},
                                                        {'.', TOK_DOT},
                                                        {';', TOK_SEMICOLON},
                                                        {'(', TOK_OPEN_BRACKET},
                                                        {')', TOK_CLOSE_BRACKET},
                                                        {',', TOK_COMMA}
                                                        };

TokenType Syntax::check_character(const char op) {
    if (s_characters.count(op))
        return s_characters.at(op);
    return TOK_INVALID;
}

TokenType Syntax::check_keyword(const std::string &word) {
    if (s_keyWords.count(word))
        return s_keyWords.at(word);
    return TOK_INVALID;
}

bool Syntax::is_delimiter(const char del) {
    return s_delimiters.count(del);
}