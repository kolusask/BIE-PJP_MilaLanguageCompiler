//
// Created by askar on 09/05/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H
#define BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H

#include "TextPosition.h"
#include "Token.h"

#include <memory>
#include <sstream>
#include <string>
#include <list>
#include <vector>


enum ExpressionType {
    EXPR_ASSIGN,
    EXPR_BINARY_OPERATION,
    EXPR_BLOCK,
    EXPR_CALL,
    EXPR_CONDITION,
    EXPR_CONST,
    EXPR_DOUBLE,
    EXPR_FOR_LOOP,
    EXPR_FUNCTION,
    EXPR_IDENTIFIER,
    EXPR_INTEGER,
    EXPR_PARENTHESES,
    EXPR_TOP_LEVEL,
    EXPR_VAR,
    EXPR_WHILE_LOOP
};

typedef std::pair<std::string, TokenType> Variable;


// Base class for all expressions in abstract syntax tree
class Expression : std::enable_shared_from_this<Expression> {
public:
    virtual std::string to_string() const = 0;
    ~Expression() {}
    virtual bool is_boolean() const { return false; }
    TextPosition position() const { return m_position; };

    virtual bool can_be_operand() const = 0;
    virtual ExpressionType type() const = 0;

protected:
    Expression(const TextPosition tp) : m_position(std::move(tp)) {}

private:
    const TextPosition m_position;
};

typedef std::shared_ptr<Expression> ExpressionPointer;


class ConstExpression : public Expression {
public:
    ConstExpression(const TextPosition tp) : Expression(std::move(tp)) {}

    void add(const std::string name, const ExpressionPointer value) {
        m_consts.push_back(std::pair<const std::string, const ExpressionPointer>(std::move(name), std::move(value)));
    }

    void add(const std::shared_ptr<ConstExpression> other) {
        m_consts.insert(m_consts.end(), other->m_consts.begin(), other->m_consts.end());
    }

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_CONST; }

    std::string to_string() const override;

    std::list<std::pair<std::string, ExpressionPointer>> consts() const { return std::move(m_consts); }

private:
    std::list<std::pair<std::string, ExpressionPointer>> m_consts;
};


class VarExpression : public Expression {
public:
    VarExpression(const TextPosition tp) : Expression(std::move(tp)) {}

    void add(const std::string name, const TokenType type) {
        m_vars.push_back(Variable(std::move(name), std::move(type)));
    }

    void add(const std::shared_ptr<VarExpression> other) {
        m_vars.insert(m_vars.end(), other->m_vars.begin(), other->m_vars.end());
    }

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_VAR; }

    std::list<Variable> vars() const { return std::move(m_vars); }

    std::string to_string() const override;

private:
    std::list<Variable> m_vars;
};


class IntegerExpression : public Expression {
public:
    IntegerExpression(const int value, const TextPosition tp) : Expression(std::move(tp)), m_value(value) {}

    bool can_be_operand() const override { return true; }
    ExpressionType type() const override { return EXPR_INTEGER; }
    int value() const { return m_value; }

    std::string to_string() const override;

private:
    const int m_value;
};


class DoubleExpression : public Expression {
public:
    DoubleExpression(const double value, const TextPosition tp) : Expression(std::move(tp)), m_value(value) {}

    bool can_be_operand() const override { return true; }
    ExpressionType type() const override { return EXPR_DOUBLE; }

    std::string to_string() const override;

private:
    const double m_value;
};


class IdentifierExpression : public Expression {
public:
    IdentifierExpression(std::string name, const TextPosition tp) :
            Expression(std::move(tp)),
            m_value(std::move(name)) {}

    bool can_be_operand() const override { return true; }
    ExpressionType type() const override { return EXPR_IDENTIFIER; }
    std::string value() const { return m_value; }

    std::string to_string() const override;

private:
    const std::string m_value;
};


class CallExpression : public Expression {
public:
    CallExpression(std::string& name, std::list<ExpressionPointer>& arguments, const TextPosition tp) :
            Expression(std::move(tp)),
            m_name(std::move(name)),
            m_arguments(std::move(arguments)) {}

    bool can_be_operand() const override { return true; }
    ExpressionType type() const override { return EXPR_CALL; }

    std::string name() const { return m_name; }
    size_t number_of_args() const { return m_arguments.size(); }
    std::list<ExpressionPointer> args() const { return std::move(m_arguments); }

    std::string to_string() const override;

private:
    const std::string m_name;
    const std::list<ExpressionPointer> m_arguments;
};

class AssignExpression : public Expression {
public:
    AssignExpression(const std::string name, const ExpressionPointer value, const TextPosition tp) :
            Expression(std::move(tp)),
            m_name(std::move(name)),
            m_value(std::move(value)) {}

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_ASSIGN; }

    std::string to_string() const override;

    std::string name() const { return std::move(m_name); }
    ExpressionPointer value() const { return std::move(m_value); }

private:
    const std::string m_name;
    const ExpressionPointer m_value;
};

// begin ... end
class BlockExpression : public Expression {
public:
    BlockExpression(std::list<ExpressionPointer>& body, const TextPosition tp) :
            Expression(std::move(tp)),
            m_body(std::move(body)) {}

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_BLOCK; }

    std::string to_string() const override;

    std::list<ExpressionPointer> body() const { return m_body; }

//    ExpressionPointer find_return(std::string funName) const {
//        ExpressionPointer result = nullptr;
//        for (const auto expr : m_body) {
//            if (expr->type() == EXPR_ASSIGN) {
//                auto assign = std::static_pointer_cast<AssignExpression>(expr);
//                if (assign->name() == funName)
//                    result = assign->value();
//            } else if (expr->type() == EXPR_BLOCK) {
//                auto candidate = std::static_pointer_cast<BlockExpression>(expr)->find_return(funName);
//                if (candidate)
//                    result = candidate;
//            }
//        }
//        return result;
//    }

private:
    const std::list<ExpressionPointer> m_body;
};


class ParenthesesExpression : public Expression {
public:
    ParenthesesExpression(const ExpressionPointer expr, const TextPosition tp) :
            Expression(std::move(tp)),
            m_expression(std::move(expr)) {}

    bool can_be_operand() const override { return true; }
    bool is_boolean() const override { return m_expression->is_boolean(); }
    ExpressionType type() const override { return EXPR_PARENTHESES; }

    std::string to_string() const override;

private:
    const ExpressionPointer m_expression;
};


class BinaryOperationExpression : public Expression {
public:
    BinaryOperationExpression(const std::shared_ptr<OperatorToken> op, const ExpressionPointer left,
                              const ExpressionPointer right, bool isBoolean, const TextPosition tp) :
            Expression(std::move(tp)),
            m_operator(std::move(op)),
            m_left(std::move(left)),
            m_right(std::move(right)),
            m_isBoolean(isBoolean) {}

    bool can_be_operand() const override { return true; }
    bool is_boolean() const override { return m_isBoolean; }
    ExpressionType type() const override { return EXPR_BINARY_OPERATION; }
    std::shared_ptr<OperatorToken> op() const { return m_operator; }
    ExpressionPointer left() const { return m_left; }
    ExpressionPointer right() const { return m_right; }

    std::string to_string() const override;

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
                       const std::shared_ptr<BlockExpression> body, const TextPosition tp) :
            Expression(std::move(tp)),
            m_name(std::move(name)),
            m_type(type),
            m_arguments(std::move(args)),
            m_consts(std::move(consts)),
            m_vars(std::move(vars)),
            m_body(std::move(body)) {}

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_FUNCTION; }
    std::string name() const { return m_name; }

    std::vector<TokenType> arg_types() const {
        std::vector<TokenType> result;
        for (auto& arg : m_arguments)
            result.push_back(arg.second);
        return std::move(result);
    }

    std::vector<std::string> arg_names() const {
        std::vector<std::string> result;
        for (auto& arg : m_arguments)
            result.push_back(arg.first);
        return std::move(result);
    }

    std::list<Variable> vars() {
        if (m_vars)
            return m_vars->vars();
        return {};
    }

    std::list<std::pair<std::string, ExpressionPointer>> consts() {
        if (m_consts)
            return m_consts->consts();
        return {};
    }

    TokenType return_type() const { return m_type; }
    std::shared_ptr<BlockExpression> body() const { return m_body; }
    //size_t number_of_args() const { return m_arguments.size(); }

    std::string to_string() const override;

private:
    const std::string m_name;
    const TokenType m_type;
    const std::list<Variable> m_arguments;
    const std::shared_ptr<ConstExpression> m_consts;
    const std::shared_ptr<VarExpression> m_vars;
    const std::shared_ptr<BlockExpression> m_body;
};


class TopLevelExpression : public Expression {
public:
    TopLevelExpression(const std::list<std::shared_ptr<FunctionExpression>> functions,
                       const std::shared_ptr<ConstExpression> consts,
                       const std::shared_ptr<VarExpression> vars,
                       const std::shared_ptr<BlockExpression> body,
                       const TextPosition tp) :
            Expression(std::move(tp)),
            m_functions(std::move(functions)),
            m_consts(std::move(consts)),
            m_vars(std::move(vars)),
            m_body(std::move(body)) {}

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_TOP_LEVEL; }

    std::string to_string() const override;

    std::list<std::pair<std::string, ExpressionPointer>> consts() const {
        if (m_consts)
            return std::move(m_consts->consts());
        return {};
    }

    std::list<Variable> vars() const {
        if (m_vars)
            return std::move(m_vars->vars());
        return {};
    }

    std::shared_ptr<BlockExpression> body() const {
        return std::move(m_body);
    }

private:
    std::list<std::shared_ptr<FunctionExpression>> m_functions;
    std::shared_ptr<ConstExpression> m_consts;
    std::shared_ptr<VarExpression> m_vars;
    std::shared_ptr<BlockExpression> m_body;
};


class ConditionExpression : public Expression {
public:
    ConditionExpression(const ExpressionPointer cond,
                        const ExpressionPointer ifTrue,
                        const ExpressionPointer ifFalse,
                        const TextPosition tp) :
            Expression(std::move(tp)),
            m_condition(std::move(cond)),
            m_ifTrue(std::move(ifTrue)),
            m_ifFalse(std::move(ifFalse)) {}

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_CONDITION; }

    ExpressionPointer condition() const { return std::move(m_condition); }
    ExpressionPointer thenBody() const { return std::move(m_ifTrue); }
    ExpressionPointer elseBody() const { return std::move(m_ifFalse); }

    std::string to_string() const override;

private:
    const ExpressionPointer m_condition;
    const ExpressionPointer m_ifTrue;
    const ExpressionPointer m_ifFalse;
};


class WhileLoopExpression : public Expression {
public:
    WhileLoopExpression(const ExpressionPointer cond, const ExpressionPointer body, const TextPosition tp) :
            Expression(std::move(tp)),
            m_condition(std::move(cond)),
            m_body(std::move(body)) {}

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_WHILE_LOOP; }

    std::string to_string() const override;

    ExpressionPointer condition() const { return m_condition; }
    ExpressionPointer body() const { return m_body; }

private:
    const ExpressionPointer m_condition;
    const ExpressionPointer m_body;
};


class ForLoopExpression : public Expression {
public:
    ForLoopExpression(const std::string counter, const ExpressionPointer start, const ExpressionPointer finish,
                      bool down, const ExpressionPointer body, const TextPosition tp) :
            Expression(std::move(tp)),
            m_counter(std::move(counter)),
            m_start(std::move(start)),
            m_finish(std::move(finish)),
            m_down(down),
            m_body(std::move(body)) {}

    bool can_be_operand() const override { return false; }
    ExpressionType type() const override { return EXPR_FOR_LOOP; }

    std::string to_string() const override;

public:
    const std::string m_counter;
    const ExpressionPointer m_start;
    const ExpressionPointer m_finish;
    const bool m_down;
    const ExpressionPointer m_body;
};



#endif //BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H
