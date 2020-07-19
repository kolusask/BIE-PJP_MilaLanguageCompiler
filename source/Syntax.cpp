//
// Created by askar on 08/05/2020.
//

#include "../include/Syntax.h"


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
                                                              {"const", TOK_CONST},
                                                              {"end", TOK_END},
                                                              {"program", TOK_PROGRAM},
                                                              {"var", TOK_VAR},
                                                              {"integer", TOK_INTEGER},
                                                              {"function", TOK_FUNCTION}};
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
    static const std::map<std::string, TokenType> operators = {{"+", TOK_PLUS},
                                                               {"-", TOK_MINUS},
                                                               {"*", TOK_MULTIPLY},
                                                               {"=", TOK_EQUAL}};
    auto it = operators.find(op);
    if (it != operators.end())
        return it->second;
    return TOK_INVALID;
}

bool Syntax::is_datatype(TokenType dt) {
    static const std::set<TokenType> dataTypes = {TOK_INTEGER};
    return dataTypes.count(dt);
}
