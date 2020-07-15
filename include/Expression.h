//
// Created by askar on 09/05/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H
#define BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H

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
#include <vector>


// Base class for all expressions in abstract syntax tree
class Expression : public std::enable_shared_from_this<Expression> {
public:
    virtual std::string to_string() const = 0;
    ~Expression() {}
    template<typename T>
    std::shared_ptr<T> as() { return std::static_pointer_cast<T>(shared_from_this()); }
    virtual llvm::Value* codegen() const { return nullptr; }

protected:
    static llvm::LLVMContext s_context;
    static llvm::IRBuilder<> s_builder;
    static std::unique_ptr<llvm::Module> s_module;
    static std::map<std::string, llvm::Value *> s_namedValues;
};

typedef std::shared_ptr<Expression> ExpressionPointer;

// key = value
class InitializationExpression : public Expression {
public:
    InitializationExpression(const std::string& key, ExpressionPointer& value):
            m_key(std::move(key)),
            m_value(std::move(value))
    {}

    virtual std::string to_string() const override;

private:
    const std::string m_key;
    ExpressionPointer m_value;
};

class LocalsExpression : public Expression {
public:
    std::string to_string() const override;

protected:
    LocalsExpression(std::vector<std::shared_ptr<InitializationExpression>>& initializations) :
            m_initializations(std::move(initializations)) {}
    virtual std::string keyword() const = 0;
    const std::vector<std::shared_ptr<InitializationExpression>> m_initializations;
};

class ConstExpression : public LocalsExpression {
public:
    ConstExpression(std::vector<std::shared_ptr<InitializationExpression>>& initializations) :
            LocalsExpression(initializations) {}

private:
    std::string keyword() const override { return "const "; }
};

class VarExpression : public LocalsExpression {
public:
    VarExpression(std::vector<std::shared_ptr<InitializationExpression>>& initializations) :
            LocalsExpression(initializations) {}

private:
    std::string keyword() const override { return "var "; }
};


class IntegerExpression : public Expression {
public:
    IntegerExpression(const int value) : m_value(value) {}
    std::string to_string() const override;
    llvm::Value* codegen() const override;

private:
    const int m_value;
};

class IdentifierExpression : public Expression {
public:
    IdentifierExpression(std::string name) : m_value(std::move(name)) {}
    std::string to_string() const override;

private:
    const std::string m_value;
};

class CallExpression : public Expression {
public:
    CallExpression(std::string& name, std::vector<ExpressionPointer>& arguments) :
            m_name(std::move(name)),
            m_arguments(std::move(arguments)) {}

    std::string to_string() const override;

private:
    const std::string m_name;
    const std::vector<ExpressionPointer> m_arguments;
};


class BlockExpression : public Expression {
public:
    BlockExpression(std::vector<ExpressionPointer>& body) : m_body(std::move(body)) {}

    std::string to_string() const override;

private:
    const std::vector<ExpressionPointer> m_body;
};

class BodyExpression : public Expression {
public:
    BodyExpression(const std::shared_ptr<ConstExpression> constExp, const std::shared_ptr<VarExpression> varExp,
                   const std::shared_ptr<BlockExpression> blkExp) :
            m_const(constExp),
            m_var(varExp),
            m_block(blkExp) {}

    std::string to_string() const override;

private:
    const std::shared_ptr<ConstExpression> m_const;
    const std::shared_ptr<VarExpression> m_var;
    const std::shared_ptr<BlockExpression> m_block;
};

class ParenthesesExpression : public Expression {
public:
    ParenthesesExpression(const ExpressionPointer expr) : m_expression(std::move(expr)) {}

    std::string to_string() const override;
private:
    const ExpressionPointer m_expression;
};

#endif //BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H