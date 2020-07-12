//
// Created by askar on 09/05/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H
#define BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H

#include "llvm/ADT/APFloat.h"
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
};

typedef std::shared_ptr<Expression> ExpressionPointer;

// key = value
class InitializationExpression : public Expression {
public:
    InitializationExpression(const std::string& key, ExpressionPointer& value):
            m_Key(std::move(key)),
            m_Value(std::move(value))
    {}

    virtual std::string to_string() const override {
        return m_Key + '=' + m_Value->to_string();
    }

private:
    const std::string m_Key;
    ExpressionPointer m_Value;
};

class LocalsExpression : public Expression {
public:
    std::string to_string() const override {
        std::ostringstream oss(keyword(), std::ios::ate);
        for (const auto& exp : m_Initializations)
            oss << exp->to_string() << ';' << std::endl;
        return oss.str();
    }

protected:
    LocalsExpression(std::vector<std::shared_ptr<InitializationExpression>>& initializations) :
            m_Initializations(std::move(initializations)) {}
    virtual std::string keyword() const = 0;
    const std::vector<std::shared_ptr<InitializationExpression>> m_Initializations;
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
    IntegerExpression(const int value) : m_Value(value) {}
    std::string to_string() const override { return std::to_string(m_Value); }

private:
    const int m_Value;
};

class IdentifierExpression : public Expression {
public:
    IdentifierExpression(std::string name) : m_Value(std::move(name)) {}
    std::string to_string() const override { return m_Value; }

private:
    const std::string m_Value;
};

class CallExpression : public Expression {
public:
    CallExpression(std::string& name, std::vector<ExpressionPointer>& arguments) :
            m_Name(std::move(name)),
            m_Arguments(std::move(arguments)) {}

    std::string to_string() const override {
        std::ostringstream oss(m_Name + '(', std::ios::ate);
        for (const auto& arg : m_Arguments)
            oss << arg->to_string();
        return oss.str() + ')';
    }

private:
    const std::string m_Name;
    const std::vector<ExpressionPointer> m_Arguments;
};


class BlockExpression : public Expression {
public:
    BlockExpression(std::vector<ExpressionPointer>& body) : m_Body(std::move(body)) {}

    std::string to_string() const override {
        std::ostringstream oss("begin\n", std::ios::ate);
        for (const auto& expr : m_Body)
            oss << expr->to_string() << ";" << std::endl;
        oss << "end";
        return oss.str();
    }

private:
    const std::vector<ExpressionPointer> m_Body;
};

class BodyExpression : public Expression {
public:
    BodyExpression(const std::shared_ptr<ConstExpression> constExp, const std::shared_ptr<VarExpression> varExp,
                   const std::shared_ptr<BlockExpression> blkExp) :
            m_Const(constExp),
            m_Var(varExp),
            m_Block(blkExp) {}

    std::string to_string() const override {
        std::ostringstream oss;
        if (m_Const)
            oss << m_Const->to_string() << std::endl;
        if (m_Var)
            oss << m_Var->to_string();
        if (m_Block)
            oss << m_Block->to_string();
        return oss.str();
    }

private:
    const std::shared_ptr<ConstExpression> m_Const;
    const std::shared_ptr<VarExpression> m_Var;
    const std::shared_ptr<BlockExpression> m_Block;
};

#endif //BIE_PJP_MILALANGUAGECOMPILER_EXPRESSION_H