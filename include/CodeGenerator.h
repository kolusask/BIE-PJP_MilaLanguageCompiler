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
    CodeGenerator(std::shared_ptr<TopLevelExpression> tree) :
            m_builder(std::make_shared<llvm::IRBuilder<>>(m_context)),
            m_module(std::make_unique<llvm::Module>("jit", m_context)),
            m_tree(std::move(tree)) {
        add_standard_functions();
    }
    llvm::Value *generate(const ExpressionPointer expr, llvm::BasicBlock *breakTo, llvm::BasicBlock *exitTo);
    llvm::Value* generate_code();
    void write_output(const char* fileName);
    void print() const;

private:
    void add_standard_functions();

    llvm::Value *gen_block(const std::shared_ptr<BlockExpression> expr, llvm::BasicBlock *breakTo,
                           llvm::BasicBlock *exitTo);
    llvm::Value* gen_integer(const std::shared_ptr<IntegerExpression> expr);
    llvm::Value* gen_double(const std::shared_ptr<DoubleExpression> expr);
    llvm::Value* gen_identifier(const std::shared_ptr<IdentifierExpression> expr);
    llvm::Value* gen_binary_operation(const std::shared_ptr<BinaryOperationExpression> expr);
    llvm::Value* gen_call(const std::shared_ptr<CallExpression> expr);
    llvm::Value* gen_function(const std::shared_ptr<FunctionExpression> expr);
    llvm::Value* gen_condition(const std::shared_ptr<ConditionExpression> expr, llvm::BasicBlock *breakTo,
                               llvm::BasicBlock *exitTo);
    llvm::Value* gen_assign(const std::shared_ptr<AssignExpression> expr);
    llvm::Value* gen_while(const std::shared_ptr<WhileLoopExpression> expr, llvm::BasicBlock *exitTo);
    llvm::Value* gen_break(llvm::BasicBlock *breakTo, TextPosition position);
    llvm::Value* gen_for(const std::shared_ptr<ForLoopExpression> expr, llvm::BasicBlock *exitTo);
    llvm::Value* gen_exit(llvm::BasicBlock* exitTo, TextPosition position);
    llvm::Value* gen_parentheses(const std::shared_ptr<ParenthesesExpression> expr);
    llvm::Value *gen_string(const std::shared_ptr<StringExpression> expr, bool newline=false);

    llvm::Value *gen_binary_ints(llvm::Value *left, llvm::Value *right, TokenType type, const TextPosition position);
    llvm::Value *gen_binary_doubles(llvm::Value *left, llvm::Value *right, TokenType type, const TextPosition position);

    llvm::Value* assign(std::string name, llvm::Value *value, TextPosition position);
    llvm::Value* load(const std::string &name, TextPosition position);
    llvm::Type* get_type(TokenType type);
    llvm::Constant* get_default_value(TokenType type);
    llvm::AllocaInst* create_alloca(llvm::Function* function, const std::string& name, llvm::Type *type);

    llvm::LLVMContext m_context;
    std::shared_ptr<llvm::IRBuilder<>> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    std::map<std::string, llvm::AllocaInst *> m_variables;
    std::map<std::string, llvm::Constant *> m_constants;
    std::map<std::string, llvm::GlobalVariable*> m_globals;
    std::shared_ptr<TopLevelExpression> m_tree;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H
