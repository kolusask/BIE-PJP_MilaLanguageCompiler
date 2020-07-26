//
// Created by askar on 20/07/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H
#define BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H

#include "Expression.h"

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

class GeneratedCode {
public:
    GeneratedCode() = default;
    GeneratedCode(const std::list<llvm::Value*> values) : m_values(std::move(values)) {}
    explicit GeneratedCode(llvm::Value* value) : m_values({value}) {}

    llvm::Value* value() const { return *m_values.begin(); }
    void add(GeneratedCode& other) {
        m_values.insert(m_values.end(), other.m_values.begin(), other.m_values.end());
    }

private:
    std::list<llvm::Value*> m_values;

};

class CodeGenerator {
public:
    CodeGenerator() :
            m_builder(std::make_shared<llvm::IRBuilder<>>(m_context)),
            m_module(std::make_unique<llvm::Module>("jit", m_context)){}
    GeneratedCode generate(const ExpressionPointer expr);
    void write_output(const char* fileName);
    void print() const;

private:
    GeneratedCode gen_integer(const std::shared_ptr<IntegerExpression> ep);
    GeneratedCode gen_identifier(const std::shared_ptr<IdentifierExpression> ep);
    GeneratedCode gen_binary_operation(const std::shared_ptr<BinaryOperationExpression> ep);
    GeneratedCode gen_call(const std::shared_ptr<CallExpression> ep);
    GeneratedCode gen_function(const std::shared_ptr<FunctionExpression> ep);
    GeneratedCode gen_condition(const std::shared_ptr<ConditionExpression> ep);
    GeneratedCode gen_assign(const std::shared_ptr<AssignExpression> ep);
    GeneratedCode gen_assign(std::string var, ExpressionPointer val, TextPosition pos);

    llvm::Type* get_type(TokenType type);
    llvm::Value* get_default_value(TokenType type);
    llvm::AllocaInst* create_alloca(llvm::Function* function, const std::string& name, llvm::Type *type);

    llvm::LLVMContext m_context;
    std::shared_ptr<llvm::IRBuilder<>> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    std::map<std::string, llvm::AllocaInst *> m_variables;
    std::map<std::string, llvm::AllocaInst *> m_constants;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H
