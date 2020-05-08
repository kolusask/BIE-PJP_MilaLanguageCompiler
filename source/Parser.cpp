//
// Created by askar on 29/04/2020.
//

#include "../include/Parser.h"

#include "../include/Exception.h"
#include "../include/Syntax.h"

Parser::Parser(std::istream &stream) : m_Lexer(stream), m_ProgramDefined(false) {}

std::shared_ptr<Token> Parser::next_token() {
    return m_Lexer.next_token();
}

void Parser::start_parsing() {
    auto token = next_token();
    if (token->type() == TOK_PROGRAM) {
        parse_program_definition();
        parse_rest(next_token());
    } else
        parse_rest(token);
}

void Parser::parse_rest(const std::shared_ptr<Token> startToken) {
    bool quit = false;
    while (!quit) {
        auto token = m_Lexer.next_token();
        switch(token->type()) {
            default: throw UnexpectedTokenException(m_Lexer.position(), Syntax::simple_token(token->type()));
            case TOK_BEGIN:
                parse_top_level();
                break;
        }
    }
}

void Parser::parse_program_definition() {

}

void Parser::parse_top_level() {

}
