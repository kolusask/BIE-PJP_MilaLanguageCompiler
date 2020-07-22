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

class GeneratedCode {};

class SingleGeneratedCode : public GeneratedCode {
public:
    SingleGeneratedCode(const llvm::Value* value) : m_value(value) {}

private:
    const llvm::Value* m_value;
};

class MultipleGeneratedCode : public GeneratedCode {
public:
    MultipleGeneratedCode(const std::list<llvm::Value*> values) : m_values(values) {}

private:
    const std::list<llvm::Value*> m_values;
};

typedef std::shared_ptr<GeneratedCode> GenResult;

class CodeGenerator {
public:
    GenResult generate(const ExpressionPointer expr);

private:
    GenResult gen_integer(const ExpressionPointer ep);
    GenResult gen_identifier(const ExpressionPointer ep);
    GenResult gen_binary_operation(const ExpressionPointer ep);
    GenResult gen_call(const ExpressionPointer ep);
    GenResult gen_function(const ExpressionPointer ep);
    GenResult gen_condition(const ExpressionPointer ep);

    llvm::Type* get_type(TokenType type);

    llvm::LLVMContext m_context;
    llvm::IRBuilder<> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    std::map<std::string, llvm::Value *> m_variables;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H
