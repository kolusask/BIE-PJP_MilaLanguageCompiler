//
// Created by askar on 09/05/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H
#define BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H

#include "Token.h"

#include <memory>
#include <sstream>
#include <string>
#include <list>


enum ExpressionType {
    EXPR_CONST,
    EXPR_VAR,
    EXPR_INTEGER,
    EXPR_IDENTIFIER,
    EXPR_CALL,
    EXPR_BLOCK,
    EXPR_PARENTHESES,
    EXPR_BINARY_OPERATION,
    EXPR_FUNCTION,
    EXPR_TOP_LEVEL,
    EXPR_CONDITION,
    EXPR_WHILE_LOOP,
    EXPR_FOR_LOOP
};

typedef std::pair<std::string, TokenType> Variable;

// Base class for all expressions in abstract syntax tree
class Expression : public std::enable_shared_from_this<Expression> {
public:
    virtual std::string to_string() const = 0;
    ~Expression() {}
    template<typename T>
    std::shared_ptr<T> as() { return std::static_pointer_cast<T>(shared_from_this()); }
    virtual bool can_be_operand() const = 0;
    virtual bool is_boolean() const { return false; }
    virtual ExpressionType type() const = 0;
};

typedef std::shared_ptr<Expression> ExpressionPointer;


class ConstExpression : public Expression {
public:
    void add(const std::string name, const ExpressionPointer value) {
        m_consts.push_back(std::pair<const std::string, const ExpressionPointer>(std::move(name), std::move(value)));
    }

    void add(const std::shared_ptr<ConstExpression> other) {
        m_consts.insert(m_consts.end(), other->m_consts.begin(), other->m_consts.end());
    }

    std::string to_string() const override;

    bool can_be_operand() const override { return false; }

    ExpressionType type() const override { return EXPR_CONST; }

private:
    std::list<std::pair<std::string, ExpressionPointer>> m_consts;
};


class VarExpression : public Expression {
public:
    void add(const std::string name, const TokenType type) {
        m_vars.push_back(Variable(std::move(name), std::move(type)));
    }

    void add(const std::shared_ptr<VarExpression> other) {
        m_vars.insert(m_vars.end(), other->m_vars.begin(), other->m_vars.end());
    }

    std::string to_string() const override;

    bool can_be_operand() const override { return false; }

    ExpressionType type() const override { return EXPR_VAR; }

private:
    std::list<Variable> m_vars;
};


class IntegerExpression : public Expression {
public:
    IntegerExpression(const int value) : m_value(value) {}
    std::string to_string() const override;

    bool can_be_operand() const override { return true; }

    ExpressionType type() const override { return EXPR_INTEGER; }

private:
    const int m_value;
};


class IdentifierExpression : public Expression {
public:
    IdentifierExpression(std::string name) : m_value(std::move(name)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return true; }

    ExpressionType type() const override { return EXPR_IDENTIFIER; }

private:
    const std::string m_value;
};


class CallExpression : public Expression {
public:
    CallExpression(std::string& name, std::list<ExpressionPointer>& arguments) :
            m_name(std::move(name)),
            m_arguments(std::move(arguments)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return true; }

    ExpressionType type() const override { return EXPR_CALL; }

private:
    const std::string m_name;
    const std::list<ExpressionPointer> m_arguments;
};

// begin ... end
class BlockExpression : public Expression {
public:
    BlockExpression(std::list<ExpressionPointer>& body) : m_body(std::move(body)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return false; }

    ExpressionType type() const override { return EXPR_BLOCK; }

private:
    const std::list<ExpressionPointer> m_body;
};


class ParenthesesExpression : public Expression {
public:
    ParenthesesExpression(const ExpressionPointer expr) : m_expression(std::move(expr)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return true; }

    bool is_boolean() const override { return m_expression->is_boolean(); }

    ExpressionType type() const override { return EXPR_PARENTHESES; }

private:
    const ExpressionPointer m_expression;
};


class BinaryOperationExpression : public Expression {
public:
    BinaryOperationExpression(const std::shared_ptr<OperatorToken> op, const ExpressionPointer left,
                              const ExpressionPointer right, bool isBoolean) :
            m_operator(std::move(op)),
            m_left(std::move(left)),
            m_right(std::move(right)),
            m_isBoolean(isBoolean) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return true; }

    bool is_boolean() const override { return m_isBoolean; }

    ExpressionType type() const override { return EXPR_BINARY_OPERATION; }

private:
    const std::shared_ptr<OperatorToken> m_operator;
    const ExpressionPointer m_left;
    const ExpressionPointer m_right;
    const bool m_isBoolean;
};

class FunctionExpression : public Expression {
public:
    FunctionExpression(const std::string name, TokenType type, const std::list<Variable> args,
                       const std::shared_ptr<ConstExpression> consts, const std::shared_ptr<VarExpression> vars,
                       const std::shared_ptr<BlockExpression> body) :
            m_name(std::move(name)),
            m_type(type),
            m_arguments(std::move(args)),
            m_consts(std::move(consts)),
            m_vars(std::move(vars)),
            m_body(std::move(body)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return false; }

    ExpressionType type() const override { return EXPR_FUNCTION; }

private:
    const std::string m_name;
    const TokenType m_type;
    const std::list<std::pair<std::string, TokenType>> m_arguments;
    const std::shared_ptr<ConstExpression> m_consts;
    const std::shared_ptr<VarExpression> m_vars;
    const std::shared_ptr<BlockExpression> m_body;
};

class TopLevelExpression : public Expression {
public:
    TopLevelExpression(const std::list<std::shared_ptr<FunctionExpression>> functions,
                       const std::shared_ptr<ConstExpression> consts,
                       const std::shared_ptr<VarExpression> vars,
                       const std::shared_ptr<BlockExpression> body) :
            m_functions(std::move(functions)),
            m_consts(std::move(consts)),
            m_vars(std::move(vars)),
            m_body(std::move(body)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return false; }

    ExpressionType type() const override { return EXPR_TOP_LEVEL; }

private:
    std::list<std::shared_ptr<FunctionExpression>> m_functions;
    std::shared_ptr<ConstExpression> m_consts;
    std::shared_ptr<VarExpression> m_vars;
    std::shared_ptr<BlockExpression> m_body;
};

class ConditionExpression : public Expression {
public:
    ConditionExpression(const ExpressionPointer cond, const ExpressionPointer ifTrue, const ExpressionPointer ifFalse) :
            m_condition(std::move(cond)),
            m_ifTrue(std::move(ifTrue)),
            m_ifFalse(std::move(ifFalse)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return false; }

    ExpressionType type() const override { return EXPR_CONDITION; }

private:
    const ExpressionPointer m_condition;
    const ExpressionPointer m_ifTrue;
    const ExpressionPointer m_ifFalse;
};

class WhileLoopExpression : public Expression {
public:
    WhileLoopExpression(const ExpressionPointer cond, const ExpressionPointer body) :
            m_condition(std::move(cond)),
            m_body(std::move(body)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return false; }

    ExpressionType type() const override { return EXPR_WHILE_LOOP; }

private:
    const ExpressionPointer m_condition;
    const ExpressionPointer m_body;
};

class ForLoopExpression : public Expression {
public:
    ForLoopExpression(const std::string counter, const ExpressionPointer start, const ExpressionPointer finish,
                      bool down, const ExpressionPointer body) :
            m_counter(std::move(counter)),
            m_start(std::move(start)),
            m_finish(std::move(finish)),
            m_down(down),
            m_body(std::move(body)) {}

    std::string to_string() const override;

    bool can_be_operand() const override { return false; }

    ExpressionType type() const override { return EXPR_FOR_LOOP; }

public:
    const std::string m_counter;
    const ExpressionPointer m_start;
    const ExpressionPointer m_finish;
    const bool m_down;
    const ExpressionPointer m_body;
};

#endif //BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H