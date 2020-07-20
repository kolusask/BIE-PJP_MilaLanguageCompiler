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
        case EXPR_CALL:
            return gen_call(std::move(expr));
        case EXPR_FUNCTION:
            return gen_function(std::move(expr));
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
    auto value = m_variables[expr->value()];
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

llvm::Value *CodeGenerator::gen_call(ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<CallExpression>(ep);
    auto function = m_module->getFunction(expr->name());
    if (!function)
        throw Exception(expr->position(), "Function is not defined");

    // Check number of arguments
    if (expr->number_of_args() != function->arg_size()) {
        TextPosition argPos = expr->position();
        argPos.column += expr->name().length() + 1;
        throw Exception(argPos,
                "Expected "
                + std::to_string(function->arg_size())
                + " arguments - got "
                + std::to_string(expr->number_of_args()));
    }

    std::vector<llvm::Value*> args;
    for (const auto& arg : expr->args())
        args.push_back(generate(arg));

    return m_builder.CreateCall(function, args, "calltmp");
}

llvm::Type *CodeGenerator::get_type(TokenType type) {
    switch (type) {
        case TOK_INTEGER:
            return llvm::Type::getDoubleTy(m_context);
        default:
            return nullptr;
    }
}

llvm::Value *CodeGenerator::gen_function(ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<FunctionExpression>(ep);
    std::vector<llvm::Type*> types;
    for (auto tt : expr->types())
        types.push_back(get_type(tt));
    auto retType = llvm::FunctionType::get(get_type(expr->return_type()), types, false);
    auto function = llvm::Function::Create(
            retType, llvm::Function::ExternalLinkage, expr->name(), m_module.get());

    auto block = llvm::BasicBlock::Create(m_context, "entry", function);
    m_builder.SetInsertPoint(block);
    for (auto& arg : function->args())
        m_variables[arg.getName()] = &arg;

    auto retVal = generate(expr->body());
    m_builder.CreateRet(retVal);
    llvm::verifyFunction(*function);

    return function;
}

