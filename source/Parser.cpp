//
// Created by askar on 29/04/2020.
//

#include "../include/Parser.h"

#include "../include/Exception.h"


Parser::Parser(std::istream &stream) : m_lexer(stream) {}

std::shared_ptr<Token> Parser::next_token() {
    return (m_lastToken = m_lexer.next_token());
}

std::string Parser::get_source() const {
    return m_source->to_string();
}

void Parser::parse() {
    std::string name = "Default";
    if (next_token()->type() == TOK_PROGRAM)
        name = parse_program_definition();
    m_source = parse_top_level();
}

std::string Parser::parse_program_definition() {
    if (next_token()->type() != TOK_IDENTIFIER)
        throw Exception(m_lexer.position(), "Expected an identifier");
    m_programName = last_token()->to_string();
    if (next_token()->type() != TOK_SEMICOLON)
        throw Exception(m_lexer.position(), "Expected ';'");
    next_token();
    return m_programName;
}

std::shared_ptr<ConstExpression> Parser::parse_const() {
    std::vector<std::shared_ptr<InitializationExpression>> expressions;
    while (next_token()->type() == TOK_IDENTIFIER) {
        std::string name = last_token()->to_string();
        if (next_token()->type() != TOK_INIT)
            throw Exception(m_lexer.position(), "Expected '='");
        next_token();
        auto value = parse_primary();
        expressions.push_back(std::make_shared<InitializationExpression>(name, value));
    }
    return std::make_shared<ConstExpression>(expressions);
}

std::shared_ptr<Token> Parser::last_token() const {
    return m_lastToken;
}

ExpressionPointer Parser::parse_top_level() {
    std::shared_ptr<ConstExpression> constExpr = nullptr;
    std::shared_ptr<VarExpression> varExpr = nullptr;
    while (last_token()->type() != TOK_EOF) {
        switch (last_token()->type()) {
            default:
                throw UnexpectedTokenException(m_lexer.position(), last_token()->to_string());
            case TOK_BEGIN:
                return std::make_shared<BodyExpression>(constExpr, varExpr, std::move(parse_block()));
            case TOK_CONST:
                constExpr = parse_const();
                break;
            case TOK_SEMICOLON:
            case TOK_EOF:
                break;
        }
    }
    return nullptr;
}

ExpressionPointer Parser::parse_primary() {
    switch(last_token()->type()) {
        default:
            throw UnexpectedTokenException(m_lexer.position(), last_token()->to_string());
        case TOK_BEGIN:
            return std::move(parse_top_level());
        case TOK_INTEGER:
            return std::move(parse_integer());
        case TOK_IDENTIFIER:
            return std::move(parse_identifier());
        case TOK_OPEN_BRACKET:
            return std::move(parse_parentheses());
        case TOK_SEMICOLON:
        case TOK_EOF:
            break;
    }
    return nullptr;
}

ExpressionPointer Parser::parse_integer() {
    int value = std::static_pointer_cast<TokInteger>(last_token())->value();
    if (next_token()->type() == TOK_SEMICOLON)
        return std::move(std::make_shared<IntegerExpression>(value));
    throw UnexpectedTokenException(m_lexer.position(), last_token()->to_string());
}

ExpressionPointer Parser::parse_identifier() {
    std::string name = std::move(last_token())->to_string();
    if (next_token()->type() == TOK_OPEN_BRACKET) {
        std::vector<ExpressionPointer> args;
        next_token();
        args.push_back(parse_primary());
        if (last_token()->type() != TOK_CLOSE_BRACKET)
            throw new Exception(m_lexer.position(), "Expected ')'");
        return std::move(std::make_shared<CallExpression>(name, args));
    }
    return std::move(std::make_shared<IdentifierExpression>(name));
}

std::shared_ptr<BlockExpression> Parser::parse_block() {
    std::vector<ExpressionPointer> body;
    while (next_token()->type() != TOK_END) {
        body.push_back(std::move(parse_primary()));
        if (next_token()->type() != TOK_SEMICOLON)
            throw new ExpectedDifferentException(m_lexer.position(), ";");
    }
    return std::move(std::make_shared<BlockExpression>(body));
}

ExpressionPointer Parser::parse_parentheses() {
    next_token();
    auto expr = parse_primary();
    if (last_token()->type() != TOK_CLOSE_BRACKET)
        throw ExpectedDifferentException(m_lexer.position(), ")");
    return std::move(std::make_shared<ParenthesesExpression>(expr));
}
