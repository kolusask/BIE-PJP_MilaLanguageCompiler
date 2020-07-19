//
// Created by askar on 29/04/2020.
//

#include "../include/Parser.h"

#include "../include/Exception.h"
#include "../include/Syntax.h"


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
        name = parse_program_name();
    m_source = parse_top_level();
}

std::string Parser::parse_program_name() {
    if (next_token()->type() != TOK_IDENTIFIER)
        throw Exception(m_lexer.position(), "Expected an identifier");
    m_programName = last_token()->to_string();
    if (next_token()->type() != TOK_SEMICOLON)
        throw ExpectedDifferentException(m_lexer.position(), ";");
    next_token();
    return m_programName;
}

std::shared_ptr<ConstExpression> Parser::parse_const() {
    auto expr = std::make_shared<ConstExpression>();
    next_token();
    while (last_token()->type() == TOK_IDENTIFIER) {
        std::string name = last_token()->to_string();
        if (next_token()->type() != TOK_EQUAL)
            throw ExpectedDifferentException(m_lexer.position(), "=");
        next_token();
        auto value = parse_expression();
        expr->add(name, value);
        if (last_token()->type() != TOK_SEMICOLON)
            throw ExpectedDifferentException(m_lexer.position(), ";");
        next_token();
    }
    return expr;
}

std::shared_ptr<VarExpression> Parser::parse_var() {
    auto expr = std::make_shared<VarExpression>();
    next_token();
    std::list<std::string> names;
    while (last_token()->type() == TOK_IDENTIFIER) {
        std::string name = last_token()->to_string();
        switch (next_token()->type()) {
            case TOK_COMMA:
                names.push_back(std::move(name));
                next_token();
                continue;
            case TOK_COLON:
                names.push_back(std::move(name));
                if (!Syntax::is_datatype(next_token()->type()))
                    throw UnexpectedTokenException(m_lexer.position(), std::move(last_token()->to_string()));
                for (const auto& n : names)
                    expr->add(n, last_token()->type());
                names.clear();
                if (next_token()->type() != TOK_SEMICOLON)
                    throw ExpectedDifferentException(m_lexer.position(), ";");
                next_token();
            default:
                return expr;
        }
    }
    return nullptr;
}

std::shared_ptr<Token> Parser::last_token() const {
    return m_lastToken;
}

std::shared_ptr<TopLevelExpression> Parser::parse_top_level() {
    std::shared_ptr<ConstExpression> constExpr = nullptr;
    std::shared_ptr<VarExpression> varExpr = nullptr;
    std::list<std::shared_ptr<FunctionExpression>> functions;
    while (last_token()->type() != TOK_EOF) {
        switch (last_token()->type()) {
            default:
                throw UnexpectedTokenException(m_lexer.position(), last_token()->to_string());
            case TOK_BEGIN: {
                auto block = std::make_shared<TopLevelExpression>(functions, constExpr, varExpr,
                                                                  std::move(parse_block()));
                if (last_token()->type() != TOK_DOT)
                    throw ExpectedDifferentException(m_lexer.position(), ".");
                return block;
            }
            case TOK_CONST:
                if (!constExpr)
                    constExpr = parse_const();
                else
                    constExpr->add(parse_const());
                break;
            case TOK_VAR:
                if (!varExpr)
                    varExpr = parse_var();
                else
                    varExpr->add(parse_var());
                break;
            case TOK_FUNCTION:
                functions.push_back(parse_function());
                break;
            case TOK_SEMICOLON:
            case TOK_EOF:
                break;
        }
    }
    return nullptr;
}

ExpressionPointer Parser::parse_expression() {
    auto expr = parse_single();
    if (expr && expr->can_be_operand())
        return parse_binary(0, expr);
    return expr;
}

std::shared_ptr<IntegerExpression> Parser::parse_integer() {
    int value = std::static_pointer_cast<IntegerToken>(last_token())->value();
    next_token();
    return std::move(std::make_shared<IntegerExpression>(value));
}

ExpressionPointer Parser::parse_identifier() {
    std::string name = std::move(last_token()->to_string());
    if (next_token()->type() == TOK_OPEN_BRACKET) {
        std::list<ExpressionPointer> args;
        do {
            next_token();
            auto arg = parse_expression();
            if (arg)
                args.push_back(arg);
        } while (last_token()->type() == TOK_COMMA);
        if (last_token()->type() != TOK_CLOSE_BRACKET)
            throw ExpectedDifferentException(m_lexer.position(), ")");
        next_token();
        return std::move(std::make_shared<CallExpression>(name, args));
    }
    return std::move(std::make_shared<IdentifierExpression>(name));
}

std::shared_ptr<BlockExpression> Parser::parse_block() {
    std::list<ExpressionPointer> body;
    next_token();
    while (last_token()->type() != TOK_END) {
        body.push_back(std::move(parse_expression()));
        if (last_token()->type() != TOK_SEMICOLON)
            throw ExpectedDifferentException(m_lexer.position(), ";");
        next_token();
    }
    next_token();
    return std::move(std::make_shared<BlockExpression>(body));
}

std::shared_ptr<ParenthesesExpression> Parser::parse_parentheses() {
    next_token();
    auto expr = parse_expression();
    if (!expr)
        throw Exception(m_lexer.position(), "Empty parentheses");
    if (last_token()->type() != TOK_CLOSE_BRACKET)
        throw ExpectedDifferentException(m_lexer.position(), ")");
    next_token();
    return std::move(std::make_shared<ParenthesesExpression>(expr));
}

ExpressionPointer Parser::parse_binary(int exprPrec, ExpressionPointer left) {;
    // Come here after reading left hand side having operator as last
    while (true) {
        int opPrec = last_token()->op_precedence();
        if (opPrec < exprPrec)
            return left;

        auto op = last_token();
        next_token();
        auto right = parse_single();

        if (opPrec < last_token()->op_precedence())
            right = parse_binary(opPrec + 1, std::move(right));

        left = std::make_shared<BinaryOperationExpression>(std::move(std::static_pointer_cast<OperatorToken>(op)),
                std::move(left), std::move(right));
    }
}

ExpressionPointer Parser::parse_single() {
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
        case TOK_CLOSE_BRACKET:
            return nullptr;
        case TOK_SEMICOLON:
        case TOK_EOF:
            break;
    }
    return ExpressionPointer();
}

std::shared_ptr<FunctionExpression> Parser::parse_function() {
    if (next_token()->type() != TOK_IDENTIFIER)
        throw Exception(m_lexer.position(), "Function name expected");
    std::string name = last_token()->to_string();

    if (next_token()->type() != TOK_OPEN_BRACKET)
        throw ExpectedDifferentException(m_lexer.position(), "(");
    std::list<Variable> args;
    while (last_token()->type() != TOK_CLOSE_BRACKET) {
        next_token();
        if (last_token()->type() != TOK_IDENTIFIER)
            throw Exception(m_lexer.position(), "Expected an argument name or ')'");
        std::string aname = last_token()->to_string();
        if (next_token()->type() != TOK_COLON)
            throw ExpectedDifferentException(m_lexer.position(), ":");
        if (!Syntax::is_datatype(next_token()->type()))
            throw Exception(m_lexer.position(), last_token()->to_string() + " is not a data type");
        TokenType atype = last_token()->type();
        args.push_back(Variable(aname, atype));
        if (next_token()->type() != TOK_COMMA && last_token()->type() != TOK_CLOSE_BRACKET)
            throw Exception(m_lexer.position(), "Expected ',' or ')");
    }

    if (next_token()->type() != TOK_COLON)
        throw ExpectedDifferentException(m_lexer.position(), ":");
    if (!Syntax::is_datatype(next_token()->type()))
        throw Exception(m_lexer.position(), last_token()->to_string() + "is not a data type");
    TokenType type = last_token()->type();
    if (next_token()->type() != TOK_SEMICOLON)
        throw ExpectedDifferentException(m_lexer.position(), ";");

    bool parsingLocals = true;
    auto consts = std::make_shared<ConstExpression>();
    auto vars = std::make_shared<VarExpression>();
    next_token();
    while (parsingLocals) {
        switch(last_token()->type()) {
            case TOK_CONST:
                consts->add(parse_const());
                break;
            case TOK_VAR:
                vars->add(parse_var());
                break;
            case TOK_BEGIN:
                parsingLocals = false;
                break;
            default:
                throw UnexpectedTokenException(m_lexer.position(), last_token()->to_string());
        }
    }

    auto body = parse_block();
    if (last_token()->type() != TOK_SEMICOLON)
        throw ExpectedDifferentException(m_lexer.position(), ";");
    next_token();
    return std::make_shared<FunctionExpression>(name, type, args, consts, vars, body);
}

