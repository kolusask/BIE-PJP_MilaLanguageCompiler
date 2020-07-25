//
// Created by askar on 20/07/2020.
//

#include "../include/CodeGenerator.h"
#include "../include/Exception.h"

GeneratedCode CodeGenerator::generate(const ExpressionPointer expr) {
    switch(expr->type()) {
        case EXPR_INTEGER:
            return std::move(gen_integer(std::move(expr)));
        case EXPR_IDENTIFIER:
            return std::move(gen_identifier(std::move(expr)));
        case EXPR_BINARY_OPERATION:
            return std::move(gen_binary_operation(std::move(expr)));
        case EXPR_CALL:
            return std::move(gen_call(std::move(expr)));
        case EXPR_FUNCTION:
            return std::move(gen_function(std::move(expr)));
        case EXPR_CONDITION:
            return std::move(gen_condition(std::move(expr)));
        default:
            throw Exception(expr->position(), "NOT IMPLEMENTED");
    }
}

GeneratedCode CodeGenerator::gen_integer(const ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<IntegerExpression>(ep);

    return std::move(GeneratedCode(llvm::ConstantInt::get(m_context, llvm::APSInt(expr->value()))));
}

GeneratedCode CodeGenerator::gen_identifier(const ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<IdentifierExpression>(ep);

    auto value = m_variables[expr->value()];
    if (!value)
        throw Exception(expr->position(), "Unknown identifier '" + expr->value() + '\'');
    return std::move(GeneratedCode(m_builder.CreateLoad(value, expr->value().c_str())));
}

GeneratedCode CodeGenerator::gen_binary_operation(ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<BinaryOperationExpression>(ep);

    auto left = generate(expr->left());
    auto right = generate(expr->right());
    switch(expr->op()->type()) {
        case TOK_PLUS:
            return std::move(GeneratedCode(m_builder.CreateFAdd(left.value(), right.value(), "addtmp")));
        case TOK_MINUS:
            return std::move(GeneratedCode(m_builder.CreateFSub(left.value(), right.value(), "subtmp")));
        case TOK_MULTIPLY:
            return std::move(GeneratedCode(m_builder.CreateFMul(left.value(), right.value(), "multmp")));
        case TOK_LESS:
            return std::move(GeneratedCode(m_builder.CreateFCmpULT(left.value(), right.value(), "cmptmp")));
        default: throw Exception(expr->position(), "NOT IMPLEMENTED");
    }

}

GeneratedCode CodeGenerator::gen_call(ExpressionPointer ep) {
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
        args.push_back(generate(arg).value());

    return std::move(GeneratedCode(m_builder.CreateCall(function, args, "calltmp")));
}

llvm::Type* CodeGenerator::get_type(TokenType type) {
    switch (type) {
        case TOK_INTEGER:
            return llvm::Type::getDoubleTy(m_context);
        default:
            return nullptr;
    }
}

GeneratedCode CodeGenerator::gen_function(ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<FunctionExpression>(ep);

    // Prototype
    std::vector<llvm::Type*> argTypes;
    for (auto& tt : expr->arg_types())
        argTypes.push_back(get_type(tt));
    auto retType = get_type(expr->return_type());
    auto functionType = llvm::FunctionType::get(retType, argTypes, false);
    auto function = llvm::Function::Create(
            functionType, llvm::Function::ExternalLinkage, expr->name(), m_module.get());
    auto argNames = expr->arg_names();
    size_t i = 0;
    for (auto& arg : function->args())
        arg.setName(argNames[i++]);

    // Body
    auto block = llvm::BasicBlock::Create(m_context, "entry", function);
    m_builder.SetInsertPoint(block);
    for (auto& arg : function->args()) {
        auto alloca = create_alloca(function, arg.getName(), arg.getType());
        m_builder.CreateStore(&arg, alloca);
        m_variables[std::string(arg.getName())] = alloca;
    }

    // TODO add return
}

llvm::AllocaInst *CodeGenerator::create_alloca(llvm::Function *function, const std::string &name, llvm::Type *type) {
    llvm::IRBuilder<> builder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return builder.CreateAlloca(type, 0, name.c_str());
}

GeneratedCode CodeGenerator::gen_condition(ExpressionPointer ep) {
    auto expr = std::static_pointer_cast<ConditionExpression>(ep);
    // if-condition
    auto condValue = m_builder.CreateFCmpONE(
            generate(expr->condition()).value(), llvm::ConstantFP::get(m_context, llvm::APFloat(0.0)));

    auto function = m_builder.GetInsertBlock()->getParent();
    // blocks
    auto thenBlock = llvm::BasicBlock::Create(m_context, "then", function);
    auto elseBlock = llvm::BasicBlock::Create(m_context, "else");
    auto mergeBlock = llvm::BasicBlock::Create(m_context, "ifcont");
    // split
    m_builder.CreateCondBr(condValue, thenBlock, elseBlock);
    // then
    m_builder.SetInsertPoint(thenBlock);
    auto thenValue = generate(expr->thenBody());
    m_builder.CreateBr(mergeBlock);
    thenBlock = m_builder.GetInsertBlock();
    // else
    function->getBasicBlockList().push_back(elseBlock);
    m_builder.SetInsertPoint(elseBlock);
    auto elseValue = generate(expr->elseBody());
    m_builder.CreateBr(mergeBlock);
    elseBlock = m_builder.GetInsertBlock();
    // merge
    function->getBasicBlockList().push_back(mergeBlock);
    m_builder.SetInsertPoint(mergeBlock);
    //auto phiNode = m_builder.CreatePHI(llvm::)
    throw Exception(expr->position(), "NOT IMPLEMENTED");
}


