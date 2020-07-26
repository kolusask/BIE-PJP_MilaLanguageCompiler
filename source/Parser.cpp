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

TextPosition Parser::position() {
    return std::move(m_lexer.position());
}

std::string Parser::get_source() const {
    return m_tree->to_string();
}

void Parser::parse() {
    std::string name = "Default";
    if (next_token()->type() == TOK_PROGRAM)
        name = parse_program_name();
    m_tree = parse_top_level();
}

std::string Parser::parse_program_name() {
    if (next_token()->type() != TOK_IDENTIFIER)
        throw Exception(std::move(position()), "Expected an identifier");
    m_programName = last_token()->to_string();
    if (next_token()->type() != TOK_SEMICOLON)
        throw ExpectedDifferentException(std::move(position()), ";");
    next_token();
    return m_programName;
}

std::shared_ptr<ConstExpression> Parser::parse_const() {
    auto expr = std::make_shared<ConstExpression>(std::move(position()));
    next_token();
    while (last_token()->type() == TOK_IDENTIFIER) {
        std::string name = last_token()->to_string();
        if (next_token()->type() != TOK_EQUAL)
            throw ExpectedDifferentException(std::move(position()), "=");
        next_token();
        auto value = parse_expression();
        expr->add(name, value);
        if (last_token()->type() != TOK_SEMICOLON)
            throw ExpectedDifferentException(std::move(position()), ";");
        next_token();
    }
    return expr;
}

std::shared_ptr<VarExpression> Parser::parse_var() {
    auto expr = std::make_shared<VarExpression>(std::move(position()));
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
                    throw UnexpectedTokenException(std::move(position()), std::move(last_token()->to_string()));
                for (const auto& n : names)
                    expr->add(n, last_token()->type());
                names.clear();
                if (next_token()->type() != TOK_SEMICOLON)
                    throw ExpectedDifferentException(std::move(position()), ";");
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
                throw UnexpectedTokenException(std::move(position()), last_token()->to_string());
            case TOK_BEGIN: {
                auto block = std::make_shared<TopLevelExpression>(functions, constExpr, varExpr,
                                                                  std::move(parse_block()), std::move(position()));
                if (last_token()->type() != TOK_DOT)
                    throw ExpectedDifferentException(std::move(position()), ".");
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
    return std::move(std::make_shared<IntegerExpression>(value, std::move(position())));
}

std::shared_ptr<DoubleExpression> Parser::parse_double() {
    double value = std::static_pointer_cast<DoubleToken>(last_token())->value();
    next_token();
    return std::move(std::make_shared<DoubleExpression>(value, std::move(position())));
}

ExpressionPointer Parser::parse_identifier() {
    std::string name = std::move(last_token()->to_string());
    if (next_token()->type() == TOK_OPEN_BRACKET) {
        // It is a function call
        std::list<ExpressionPointer> args;
        next_token();
        while (last_token()->type() != TOK_CLOSE_BRACKET) {
            auto arg = parse_expression();
            if (!arg->can_be_operand())
                throw Exception(std::move(position()), "Not a valid function argument");
            args.push_back(arg);
            if (last_token()->type() == TOK_COMMA)
                next_token();
        }
        next_token();
        return std::move(std::make_shared<CallExpression>(name, args, std::move(position())));
    }
    return std::move(std::make_shared<IdentifierExpression>(name, std::move(position())));
}

std::shared_ptr<BlockExpression> Parser::parse_block() {
    std::list<ExpressionPointer> body;
    next_token();
    while (last_token()->type() != TOK_END) {
        body.push_back(std::move(parse_expression()));
        if (last_token()->type() != TOK_SEMICOLON)
            throw ExpectedDifferentException(std::move(position()), ";");
        next_token();
    }
    next_token();
    return std::move(std::make_shared<BlockExpression>(body, std::move(position())));
}

std::shared_ptr<ParenthesesExpression> Parser::parse_parentheses() {
    next_token();
    auto expr = parse_expression();
    if (!expr)
        throw Exception(std::move(position()), "Empty parentheses");
    if (last_token()->type() != TOK_CLOSE_BRACKET)
        throw ExpectedDifferentException(std::move(position()), ")");
    next_token();
    return std::move(std::make_shared<ParenthesesExpression>(expr, std::move(position())));
}

ExpressionPointer Parser::parse_binary(int exprPrec, ExpressionPointer left) {;
    std::shared_ptr<Token> op = nullptr;
    std::shared_ptr<Expression> right = nullptr;
    // Come here after reading left hand side having operator as last
    while (true) {
        int opPrec = last_token()->op_precedence();
        if (opPrec < exprPrec) {
            if (op && op->type() == TOK_ASSIGN) {
                auto expr = std::static_pointer_cast<BinaryOperationExpression>(left);
                if (expr->left()->type() == EXPR_IDENTIFIER) {
                    auto assignee = std::static_pointer_cast<IdentifierExpression>(expr->left());
                    return std::make_shared<AssignExpression>(assignee->value(), right, position());
                }
                throw Exception(left->position(), "Left operand of assignment must be a variable name");
            }
            return left;
        }

        op = last_token();
        next_token();
        right = parse_single();

        if (opPrec < last_token()->op_precedence())
            right = parse_binary(opPrec + 1, std::move(right));

        left = std::make_shared<BinaryOperationExpression>(std::move(std::static_pointer_cast<OperatorToken>(op)),
                std::move(left), right, Syntax::is_bool_operator(op->type()), std::move(position()));
    }
}

ExpressionPointer Parser::parse_single() {
    switch(last_token()->type()) {
        default:
            throw UnexpectedTokenException(std::move(position()), last_token()->to_string());
        case TOK_BEGIN:
            return std::move(parse_block());
        case TOK_INTEGER:
            return std::move(parse_integer());
        case TOK_DOUBLE:
            return std::move(parse_double());
        case TOK_IDENTIFIER:
            return std::move(parse_identifier());
        case TOK_OPEN_BRACKET:
            return std::move(parse_parentheses());
        case TOK_IF:
            return std::move(parse_condition());
        case TOK_WHILE:
            return std::move(parse_while());
        case TOK_FOR:
            return std::move(parse_for());
        case TOK_MINUS:
            return std::move(parse_minus());
        case TOK_SEMICOLON:
        case TOK_EOF:
            break;
    }
    return ExpressionPointer();
}

std::shared_ptr<FunctionExpression> Parser::parse_function() {
    if (next_token()->type() != TOK_IDENTIFIER)
        throw Exception(std::move(position()), "Function name expected");
    std::string name = last_token()->to_string();

    if (next_token()->type() != TOK_OPEN_BRACKET)
        throw ExpectedDifferentException(std::move(position()), "(");
    std::list<Variable> args;
    while (last_token()->type() != TOK_CLOSE_BRACKET) {
        next_token();
        if (last_token()->type() != TOK_IDENTIFIER)
            throw Exception(std::move(position()), "Expected an argument name or ')'");
        std::string aname = last_token()->to_string();
        if (next_token()->type() != TOK_COLON)
            throw ExpectedDifferentException(std::move(position()), ":");
        if (!Syntax::is_datatype(next_token()->type()))
            throw Exception(std::move(position()), last_token()->to_string() + " is not a data type");
        TokenType atype = last_token()->type();
        args.push_back(Variable(aname, atype));
        if (next_token()->type() != TOK_COMMA && last_token()->type() != TOK_CLOSE_BRACKET)
            throw Exception(std::move(position()), "Expected ',' or ')");
    }

    if (next_token()->type() != TOK_COLON)
        throw ExpectedDifferentException(std::move(position()), ":");
    if (!Syntax::is_datatype(next_token()->type()))
        throw Exception(std::move(position()), last_token()->to_string() + "is not a data type");
    TokenType type = last_token()->type();
    if (next_token()->type() != TOK_SEMICOLON)
        throw ExpectedDifferentException(std::move(position()), ";");

    bool parsingLocals = true;
    auto consts = std::make_shared<ConstExpression>(std::move(position()));
    auto vars = std::make_shared<VarExpression>(std::move(position()));
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
                throw UnexpectedTokenException(std::move(position()), last_token()->to_string());
        }
    }

    auto body = parse_block();
    if (last_token()->type() != TOK_SEMICOLON)
        throw ExpectedDifferentException(std::move(position()), ";");
    next_token();
    return std::make_shared<FunctionExpression>(name, type, args, consts, vars, body, std::move(position()));
}

std::shared_ptr<ConditionExpression> Parser::parse_condition() {
    next_token();
    auto condition = parse_expression();
    if (!condition->is_boolean())
        throw Exception(std::move(position()), "Condition must be a boolean expression");
    if (last_token()->type() != TOK_THEN)
        throw ExpectedDifferentException(std::move(position()), "then");
    next_token();
    auto ifTrue = parse_expression();
    ExpressionPointer ifFalse = nullptr;
    if (last_token()->type() == TOK_ELSE) {
        next_token();
        ifFalse = parse_expression();
    }
    if (last_token()->type() != TOK_SEMICOLON)
        throw ExpectedDifferentException(std::move(position()), ";");
    return std::make_shared<ConditionExpression>(condition, ifTrue, ifFalse, std::move(position()));
}

std::shared_ptr<WhileLoopExpression> Parser::parse_while() {
    next_token();
    auto condition = parse_expression();
    if (!condition->is_boolean())
        throw Exception(std::move(position()), "Condition must be a boolean expression");
    if (last_token()->type() != TOK_DO)
        throw ExpectedDifferentException(std::move(position()), "do");
    next_token();
    auto body = parse_expression();
    if (last_token()->type() != TOK_SEMICOLON)
        throw ExpectedDifferentException(std::move(position()), ";");
    return std::make_shared<WhileLoopExpression>(condition, body, std::move(position()));
}

std::shared_ptr<ForLoopExpression> Parser::parse_for() {
    if (next_token()->type() != TOK_IDENTIFIER)
        throw Exception(std::move(position()), "Expected a counter variable name");
    std::string counter = last_token()->to_string();
    if (next_token()->type() != TOK_ASSIGN)
        throw ExpectedDifferentException(std::move(position()), ":=");

    next_token();
    auto start = parse_expression();
    if (!start->can_be_operand())
        throw Exception(std::move(position()), "Invalid starting value");

    bool downto;
    if (last_token()->type() == TOK_TO)
        downto = false;
    else if (last_token()->type() == TOK_DOWNTO)
        downto = true;
    else
        throw Exception(std::move(position()), "Expected 'to' or 'downto'");

    next_token();
    auto finish = parse_expression();
    if (!finish->can_be_operand())
        throw Exception(std::move(position()), "Invalid final value");

    if (last_token()->type() != TOK_DO)
        throw ExpectedDifferentException(std::move(position()), "do");

    next_token();
    auto body = parse_expression();
    return std::make_shared<ForLoopExpression>(counter, start, finish, downto, body, std::move(position()));
}

std::shared_ptr<BinaryOperationExpression> Parser::parse_minus() {
    next_token();
    static const auto multiply = std::make_shared<OperatorToken>(TOK_MULTIPLY);
    static const auto minusOne = std::make_shared<IntegerExpression>(-1, std::move(position()));
    auto expr = parse_expression();
    return std::make_shared<BinaryOperationExpression>(multiply, minusOne, expr, false, std::move(position()));
}

std::shared_ptr<TopLevelExpression> Parser::get_tree() const {
    return m_tree;
}

