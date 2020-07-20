//
// Created by askar on 20/07/2020.
//

#include "../include/CodeGenerator.h"
#include "../include/Exception.h"

llvm::Value *CodeGenerator::generate(const ExpressionPointer expr) {
    switch(expr->type()) {
        case EXPR_INTEGER:
            return gen_integer(std::move(expr));
        case EXPR_IDENTIFIER:
            return gen_identifier(std::move(expr));
        case EXPR_BINARY_OPERATION:
            return gen_binary_operation(std::move(expr));
        default:
            return nullptr;
    }
}

llvm::Value *CodeGenerator::gen_integer(const ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<IntegerExpression>(ep);
    return llvm::ConstantInt::get(m_context, llvm::APSInt(expr->value()));
}

llvm::Value *CodeGenerator::gen_identifier(const ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<IdentifierExpression>(ep);
    auto value = m_namedValues[expr->value()];
    if (!value)
        throw Exception(expr->position(), "Unknown identifier '" + expr->value() + '\'');
    return value;
}

llvm::Value *CodeGenerator::gen_binary_operation(ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<BinaryOperationExpression>(ep);
    auto left = generate(expr->left());
    auto right = generate(expr->right());
    switch(expr->op()->type()) {
        case TOK_PLUS:
            return m_builder.CreateFAdd(left, right, "addtmp");
        case TOK_MINUS:
            return m_builder.CreateFSub(left, right, "subtmp");
        case TOK_MULTIPLY:
            return m_builder.CreateFMul(left, right, "multmp");
        case TOK_LESS:
            return m_builder.CreateFCmpULT(left, right, "cmptmp");
        default: return nullptr;
    }

}
