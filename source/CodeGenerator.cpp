//
// Created by askar on 20/07/2020.
//

#include "../include/CodeGenerator.h"
#include "../include/Exception.h"

llvm::Value *CodeGenerator::generate(ExpressionPointer expr) {
    switch(expr->type()) {
        case EXPR_INTEGER:
            return gen_integer(expr);
        default:
            return nullptr;
    }
}

llvm::Value *CodeGenerator::gen_integer(ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<IntegerExpression>(ep);
    return llvm::ConstantInt::get(m_context, llvm::APSInt(expr->value()));
}

llvm::Value *CodeGenerator::gen_identifier(ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<IdentifierExpression>(ep);
    auto value = m_namedValues[expr->value()];
    if (!value)
        throw Exception(expr->position(), "Unknown identifier '" + expr->value() + '\'');
    return value;
}
