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
    GeneratedCode(const std::list<llvm::Value*> values) : m_values(std::move(values)) {}
    explicit GeneratedCode(llvm::Value* value) : m_values({value}) {}

    llvm::Value* value() const { return *m_values.begin(); }

private:
    const std::list<llvm::Value*> m_values;

};

class CodeGenerator {
public:
    GeneratedCode generate(const ExpressionPointer expr);

private:
    GeneratedCode gen_integer(const ExpressionPointer ep);
    GeneratedCode gen_identifier(const ExpressionPointer ep);
    GeneratedCode gen_binary_operation(const ExpressionPointer ep);
    GeneratedCode gen_call(const ExpressionPointer ep);
    GeneratedCode gen_function(const ExpressionPointer ep);
    GeneratedCode gen_condition(const ExpressionPointer ep);
    GeneratedCode gen_assign(const ExpressionPointer ep);

    llvm::Type* get_type(TokenType type);
    llvm::AllocaInst* create_alloca(llvm::Function* function, const std::string& name, llvm::Type *type);

    llvm::LLVMContext m_context;
    llvm::IRBuilder<> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    std::map<std::string, llvm::AllocaInst *> m_variables;
    std::map<std::string, llvm::AllocaInst *> m_constants;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H
