//
// Created by askar on 29/04/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_PARSER_H
#define BIE_PJP_MILALANGUAGECOMPILER_PARSER_H

#include "Lexer.h"

#include "Expression.h"

#include <list>


class Parser {
public:
    Parser(std::istream& stream);
    void parse();
    std::string get_source() const;

private:
    std::string parse_program_name();
    ExpressionPointer parse_expression();
    ExpressionPointer parse_single();
    // parse specific constructs
    ExpressionPointer parse_binary(int exprPrec, ExpressionPointer left);
    ExpressionPointer parse_identifier();   // or a function call
    std::shared_ptr<IntegerExpression> parse_integer();
    std::shared_ptr<DoubleExpression> parse_double();
    std::shared_ptr<TopLevelExpression> parse_top_level();
    std::shared_ptr<ConstExpression> parse_const();
    std::shared_ptr<VarExpression> parse_var();
    std::shared_ptr<BlockExpression> parse_block();
    std::shared_ptr<ParenthesesExpression> parse_parentheses();
    std::shared_ptr<FunctionExpression> parse_function();
    std::shared_ptr<ConditionExpression> parse_condition();
    std::shared_ptr<WhileLoopExpression> parse_while();
    std::shared_ptr<ForLoopExpression> parse_for();

    inline std::shared_ptr<Token> last_token() const;
    std::shared_ptr<Token> next_token();

    TextPosition position();

    std::shared_ptr<Token> m_lastToken;
    Lexer m_lexer;
    std::string m_programName = "";
    ExpressionPointer m_source = nullptr;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_PARSER_H