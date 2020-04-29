//
// Created by askar on 29/04/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_PARSER_H
#define BIE_PJP_MILALANGUAGECOMPILER_PARSER_H

#include "include/Lexer.h"

class Parser {
public:
    Parser(std::istream& stream);
    void start_parsing();

    // parse specific constructs
    void parse_program_definition();
    void parse_top_level();
    void parse_identifier();

private:
    std::shared_ptr<Token> next_token();
    void parse_rest(const std::shared_ptr<Token> startToken);
    Lexer m_Lexer;
    bool m_ProgramDefined;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_PARSER_H
