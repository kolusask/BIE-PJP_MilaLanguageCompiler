//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_LEXER_H
#define MILALANGUAGECOMPILER_LEXER_H

#include "../include/Token.h"

#include <istream>
#include <map>
#include <memory>
#include <set>

class Lexer {
public:
    Lexer(std::istream& stream);
    std::shared_ptr<Token> next_token();

private:
    int read_char();
    int read_number();
    std::string read_identifier();
    std::istream& m_Stream;
    int m_Char;
    size_t m_Line = 1;
    size_t m_Col = 0;

};


#endif //MILALANGUAGECOMPILER_LEXER_H
