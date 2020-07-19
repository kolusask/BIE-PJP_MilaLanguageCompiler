//
// Created by askar on 09/05/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H
#define BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H

#include "Token.h"

#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <memory>
#include <sstream>
#include <string>
#include <list>


typedef std::pair<std::string, TokenType> Variable;

// Base class for all expressions in abstract syntax tree
class Expression : public std::enable_shared_from_this<Expression> {
public:
    virtual std::string to_string() const = 0;
    ~Expression() {}
    template<typename T>
    std::shared_ptr<T> as() { return std::static_pointer_cast<T>(shared_from_this()); }
    virtual llvm::Value* codegen() const { return nullptr; }
    virtual bool can_be_operand() const = 0;

protected:
    static llvm::LLVMContext s_context;
    static llvm::IRBuilder<> s_builder;
    static std::unique_ptr<llvm::Module> s_module;
    static std::map<std::string, llvm::Value *> s_namedValues;
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

private:
    std::list<Variable> m_vars;
};


class IntegerExpression : public Expression {
public:
    IntegerExpression(const int value) : m_value(value) {}
    std::string to_string() const override;
    llvm::Value* codegen() const override;
    bool can_be_operand() const override { return true; }

private:
    const int m_value;
};

class IdentifierExpression : public Expression {
public:
    IdentifierExpression(std::string name) : m_value(std::move(name)) {}

    std::string to_string() const override;
    bool can_be_operand() const override { return true; }

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

private:
    const std::list<ExpressionPointer> m_body;
};

/*// const ... var ... begin ... begin ... end ... end
class BodyExpression : public Expression {
public:
    BodyExpression(const std::shared_ptr<ConstExpression> constExp, const std::shared_ptr<VarExpression> varExp,
                   const std::shared_ptr<BlockExpression> blkExp) :
            m_const(constExp),
            m_var(varExp),
            m_block(blkExp) {}

    std::string to_string() const override;
    bool can_be_operand() const override { return false; }

private:
    const std::shared_ptr<ConstExpression> m_const;
    const std::shared_ptr<VarExpression> m_var;
    const std::shared_ptr<BlockExpression> m_block;
};*/

class ParenthesesExpression : public Expression {
public:
    ParenthesesExpression(const ExpressionPointer expr) : m_expression(std::move(expr)) {}

    std::string to_string() const override;
    bool can_be_operand() const override { return true; }

private:
    const ExpressionPointer m_expression;
};

class BinaryOperationExpression : public Expression {
public:
    BinaryOperationExpression(const std::shared_ptr<OperatorToken> op, const ExpressionPointer left,
                              const ExpressionPointer right) :
            m_operator(std::move(op)),
            m_left(std::move(left)),
            m_right(std::move(right)) {}

    std::string to_string() const override;
    bool can_be_operand() const override { return false; }

private:
    const std::shared_ptr<OperatorToken> m_operator;
    const ExpressionPointer m_left;
    const ExpressionPointer m_right;
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

private:
    std::list<std::shared_ptr<FunctionExpression>> m_functions;
    std::shared_ptr<ConstExpression> m_consts;
    std::shared_ptr<VarExpression> m_vars;
    std::shared_ptr<BlockExpression> m_body;
};

#endif //BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H