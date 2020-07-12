//
// Created by askar on 29/04/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_PARSER_H
#define BIE_PJP_MILALANGUAGECOMPILER_PARSER_H

#include "Lexer.h"

#include "Expression.h"

#include <vector>


class Parser {
public:
    Parser(std::istream& stream);
    void parse();
    std::string get_source() const;

private:
    // parse specific constructs
    std::string parse_program_definition();
    ExpressionPointer parse_primary();
    ExpressionPointer parse_top_level();
    ExpressionPointer parse_identifier();
    ExpressionPointer parse_integer();
    std::shared_ptr<ConstExpression> parse_const();
    std::shared_ptr<BlockExpression> parse_block();
    ExpressionPointer parse_brackets();

    inline std::shared_ptr<Token> last_token() const;
    std::shared_ptr<Token> next_token();
    void parse_rest();

    std::shared_ptr<Token> m_lastToken;
    Lexer m_lexer;
    std::string m_programName = "";
    ExpressionPointer m_source = nullptr;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_PARSER_H