//
// Created by askar on 20/07/2020.
//

#include "../include/CodeGenerator.h"
#include "../include/Exception.h"

#include "llvm/IR/LegacyPassManager.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

llvm::Value* CodeGenerator::generate(const ExpressionPointer expr) {
    switch(expr->type()) {
        case EXPR_INTEGER:
            return std::move(gen_integer(std::move(std::static_pointer_cast<IntegerExpression>(expr))));
        case EXPR_IDENTIFIER:
            return std::move(gen_identifier(std::move(std::static_pointer_cast<IdentifierExpression>(expr))));
        case EXPR_BINARY_OPERATION:
            return std::move(gen_binary_operation(std::move(std::static_pointer_cast<BinaryOperationExpression>(expr))));
        case EXPR_CALL:
            return std::move(gen_call(std::move(std::static_pointer_cast<CallExpression>(expr))));
        case EXPR_FUNCTION:
            return std::move(gen_function(std::move(std::static_pointer_cast<FunctionExpression>(expr))));
        case EXPR_CONDITION:
            return std::move(gen_condition(std::move(std::static_pointer_cast<ConditionExpression>(expr))));
        case EXPR_ASSIGN:
            return std::move(gen_assign(std::move(std::static_pointer_cast<AssignExpression>(expr))));
        default:
            throw Exception(expr->position(), "NOT IMPLEMENTED");
    }
}

llvm::Value* CodeGenerator::gen_integer(const std::shared_ptr<IntegerExpression> ep) {
    auto expr = std::static_pointer_cast<IntegerExpression>(ep);

    return llvm::ConstantInt::get(m_context, llvm::APSInt(expr->value()));
}

llvm::Value* CodeGenerator::gen_identifier(const std::shared_ptr<IdentifierExpression> ep) {
    auto expr = std::static_pointer_cast<IdentifierExpression>(ep);

    auto value = m_variables[expr->value()];
    if (!value)
        value = m_constants[expr->value()];
    if (!value)
        throw Exception(expr->position(), "Unknown identifier '" + expr->value() + '\'');
    return m_builder->CreateLoad(value, expr->value().c_str());
}

llvm::Value* CodeGenerator::gen_binary_operation(const std::shared_ptr<BinaryOperationExpression> ep) {
    auto expr = std::static_pointer_cast<BinaryOperationExpression>(ep);

    auto left = generate(expr->left());
    auto right = generate(expr->right());
    switch(expr->op()->type()) {
        case TOK_PLUS:
            return m_builder->CreateFAdd(left, right, "addtmp");
        case TOK_MINUS:
            return m_builder->CreateFSub(left, right, "subtmp");
        case TOK_MULTIPLY:
            return m_builder->CreateFMul(left, right, "multmp");
        case TOK_LESS:
            return m_builder->CreateFCmpULT(left, right, "cmptmp");
        default: throw Exception(expr->position(), "NOT IMPLEMENTED");
    }

}

llvm::Value* CodeGenerator::gen_call(const std::shared_ptr<CallExpression> ep) {
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

    return m_builder->CreateCall(function, args, "calltmp");
}

llvm::Type* CodeGenerator::get_type(TokenType type) {
    switch (type) {
        case TOK_INTEGER:
            return llvm::Type::getDoubleTy(m_context);
        default:
            return nullptr;
    }
}

llvm::Value *CodeGenerator::get_default_value(TokenType type) {
    switch (type) {
        case TOK_INTEGER: {
            static auto intZero = llvm::ConstantInt::get(m_context, llvm::APSInt(0));
            return intZero;
        }
        default:
            return nullptr;
    }
}

llvm::Value* CodeGenerator::gen_function(const std::shared_ptr<FunctionExpression> ep) {
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
    m_builder->SetInsertPoint(block);
    for (auto& arg : function->args()) {
        auto alloca = create_alloca(function, arg.getName(), arg.getType());
        m_builder->CreateStore(&arg, alloca);
        m_variables[std::string(arg.getName())] = alloca;
    }

    // TODO add return
    return nullptr;
}

llvm::AllocaInst *CodeGenerator::create_alloca(llvm::Function *function, const std::string &name, llvm::Type *type) {
    llvm::IRBuilder<> builder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return builder.CreateAlloca(type, 0, name.c_str());
}

llvm::Value* CodeGenerator::gen_condition(const std::shared_ptr<ConditionExpression> ep) {
    auto expr = std::static_pointer_cast<ConditionExpression>(ep);
    // if-condition
    auto condValue = m_builder->CreateFCmpONE(
            generate(expr->condition()), llvm::ConstantFP::get(m_context, llvm::APFloat(0.0)));

    auto function = m_builder->GetInsertBlock()->getParent();
    // blocks
    auto thenBlock = llvm::BasicBlock::Create(m_context, "then", function);
    auto elseBlock = llvm::BasicBlock::Create(m_context, "else");
    auto mergeBlock = llvm::BasicBlock::Create(m_context, "ifcont");
    // split
    m_builder->CreateCondBr(condValue, thenBlock, elseBlock);
    // then
    m_builder->SetInsertPoint(thenBlock);
    auto thenValue = generate(expr->thenBody());
    m_builder->CreateBr(mergeBlock);
    thenBlock = m_builder->GetInsertBlock();
    // else
    function->getBasicBlockList().push_back(elseBlock);
    m_builder->SetInsertPoint(elseBlock);
    auto elseValue = generate(expr->elseBody());
    m_builder->CreateBr(mergeBlock);
    elseBlock = m_builder->GetInsertBlock();
    // merge
    function->getBasicBlockList().push_back(mergeBlock);
    m_builder->SetInsertPoint(mergeBlock);
    //m_builder->
}

llvm::Value* CodeGenerator::gen_assign(const std::shared_ptr<AssignExpression> ep) {
    auto expr = std::static_pointer_cast<AssignExpression>(ep);

    auto value = generate(expr->value());
    auto variable = m_variables[expr->name()];
    if (!variable)
        throw Exception(expr->position(), "Unknown identifier: '" + expr->name() + '\'');
    m_builder->CreateStore(value, variable);
    return value;
}

void CodeGenerator::write_output(const char *fileName) {
    // Initialize the target registry etc
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    m_module->setTargetTriple(targetTriple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

    if (!target)
        throw Exception(error);

    auto cpu = "generic";
    auto features = "";

    llvm::TargetOptions opt;
    auto relocModel = llvm::Optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, relocModel);

    m_module->setDataLayout(targetMachine->createDataLayout());

    std::error_code errorCode;
    llvm::raw_fd_ostream dest(fileName, errorCode, llvm::sys::fs::OF_None);

    if (errorCode)
        throw Exception(errorCode.message());

    llvm::legacy::PassManager pass;

    auto fileType = llvm::CGFT_ObjectFile;

    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType))
        throw Exception("I don't want to do that");

    pass.run(*m_module);

    dest.flush();
}

void CodeGenerator::print() const {
    m_module->print(llvm::errs(), nullptr);
}



//llvm::Value* CodeGenerator::gen_assign(const std::string &name, ExpressionPointer value) {
//    return llvm::Value*(__cxx11::list());
//}


