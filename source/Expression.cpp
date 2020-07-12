//
// Created by askar on 12/07/2020.
//

#include "../include/Expression.h"

llvm::LLVMContext Expression::s_context;
llvm::IRBuilder<> Expression::s_builder(Expression::s_context);
std::unique_ptr<llvm::Module> Expression::s_module =
        std::make_unique<llvm::Module>("JIT Module", Expression::s_context);
std::map<std::string, llvm::Value *> Expression::s_namedValues;

llvm::Value *IntegerExpression::codegen() const {
    return llvm::ConstantInt::get(s_context, llvm::APSInt(m_Value));
}
