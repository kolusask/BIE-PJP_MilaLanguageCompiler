//
// Created by askar on 29/04/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_PARSER_H
#define BIE_PJP_MILALANGUAGECOMPILER_PARSER_H

#include "include/Lexer.h"

class Parser {
public:
    Parser(std::istream& stream);
    void parse();



private:
    Lexer m_Lexer;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_PARSER_H
