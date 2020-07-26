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

class CodeGenerator {
public:
    CodeGenerator() :
            m_builder(std::make_shared<llvm::IRBuilder<>>(m_context)),
            m_module(std::make_unique<llvm::Module>("jit", m_context)){}
    llvm::Value* generate(const ExpressionPointer expr);
    void write_output(const char* fileName);
    void print() const;

private:

    llvm::Value* gen_integer(const std::shared_ptr<IntegerExpression> ep);
    llvm::Value* gen_identifier(const std::shared_ptr<IdentifierExpression> ep);
    llvm::Value* gen_binary_operation(const std::shared_ptr<BinaryOperationExpression> ep);
    llvm::Value* gen_call(const std::shared_ptr<CallExpression> ep);
    llvm::Value* gen_function(const std::shared_ptr<FunctionExpression> ep);
    llvm::Value* gen_condition(const std::shared_ptr<ConditionExpression> ep);
    llvm::Value* gen_assign(const std::shared_ptr<AssignExpression> ep);
    llvm::Value* gen_assign(std::string var, ExpressionPointer val, TextPosition pos);

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
