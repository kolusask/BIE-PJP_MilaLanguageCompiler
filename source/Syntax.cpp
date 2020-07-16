//
// Created by askar on 08/05/2020.
//

#include "../include/Syntax.h"


TokenType Syntax::check_character(const char ch) {
    static const std::map<char, TokenType> characters = {{'=', TOK_INIT},
                                                         {'.', TOK_DOT},
                                                         {';', TOK_SEMICOLON},
                                                         {'(', TOK_OPEN_BRACKET},
                                                         {')', TOK_CLOSE_BRACKET},
                                                         {',', TOK_COMMA}};
    auto it = characters.find(ch);
    if (it != characters.end())
        return it->second;
    return TOK_INVALID;
}

TokenType Syntax::check_keyword(const std::string &word) {
    static const std::map<std::string, TokenType> keyWords = {{"begin", TOK_BEGIN},
                                                              {"const", TOK_CONST},
                                                              {"end", TOK_END},
                                                              {"program", TOK_PROGRAM}};
    auto it = keyWords.find(word);
    if (it != keyWords.end())
        return it->second;
    return TOK_INVALID;
}

bool Syntax::is_delimiter(const char del) {
    static const std::set<char> delimiters = { ' ',
                                               '\n',
                                               '\t' };
    return s_delimiters.count(del);
}
