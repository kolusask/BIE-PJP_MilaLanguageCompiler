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

//#include <cstdlib>

llvm::Value* CodeGenerator::generate(const ExpressionPointer expr) {
    auto type = expr->type();
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
        case EXPR_BLOCK:
            return std::move(gen_block(std::move(std::static_pointer_cast<BlockExpression>(expr))));
        default:
            throw Exception("NOT IMPLEMENTED");
    }
}

llvm::Value* CodeGenerator::gen_integer(const std::shared_ptr<IntegerExpression> ep) {
    auto expr = std::static_pointer_cast<IntegerExpression>(ep);

    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), expr->value());// llvm::ConstantInt::get(m_context, llvm::APSInt(expr->value()));
}

llvm::Value* CodeGenerator::gen_identifier(const std::shared_ptr<IdentifierExpression> expr) {
    llvm::Value* value;
    if ((value = m_constants[expr->value()]))
        return value;
    if ((value = m_variables[expr->value()]))
        return m_builder->CreateLoad(value, expr->value().c_str());

    throw Exception(expr->position(), "Unknown identifier '" + expr->value() + '\'');
}

//void CodeGenerator::to_double(llvm::Value *&value) {
//    value->getType() ==
//}
//
//void CodeGenerator::equalize_types(llvm::Value *left, llvm::Value *right) {
//
//}

llvm::Value* CodeGenerator::gen_binary_operation(const std::shared_ptr<BinaryOperationExpression> ep) {
    auto expr = std::static_pointer_cast<BinaryOperationExpression>(ep);

    auto left = generate(expr->left());
    auto right = generate(expr->right());

    switch(expr->op()->type()) {
        case TOK_PLUS:
            return m_builder->CreateAdd(left, right, "addtmp");
        case TOK_MINUS:
            return m_builder->CreateSub(left, right, "subtmp");
        case TOK_MULTIPLY:
            return m_builder->CreateMul(left, right, "multmp");
        case TOK_LESS:
            return m_builder->CreateICmpSLT(left, right, "cmptmp");
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

    std::vector<llvm::Value *> args;
    if (expr->name() == "readln") {
        auto arg = *expr->args().cbegin();
        if (arg->type() == EXPR_IDENTIFIER) {
            auto ident = std::static_pointer_cast<IdentifierExpression>(arg);
            auto value = m_variables[ident->value()];
            if (!value)
                throw Exception(arg->position(), "Unknown or constant identifier");
            args.push_back(value);
        } else
            throw Exception(arg->position(), "Can only read into a variable");
    } else {
        for (const auto &arg : expr->args())
            args.push_back(generate(arg));
    }
    auto call = m_builder->CreateCall(function, args, "calltmp");
    if (expr->name() == "readln")
        m_builder->CreateStore(
                llvm::ConstantInt::get(llvm::Type::getInt8Ty(m_context), 0), m_variables["_extra"]);
    return call;
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

llvm::Value* CodeGenerator::gen_function(const std::shared_ptr<FunctionExpression> expr) {
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
//TODO Clean up this mess
    // arguments
    for (auto& arg : function->args()) {
        auto alloca = create_alloca(function, arg.getName(), arg.getType());
        m_builder->CreateStore(&arg, alloca);
        m_variables[std::string(arg.getName())] = alloca;
    }

    // Vars and consts
    auto oldConsts = m_constants;
    for (auto& c : expr->consts())
        m_constants[c.first] = llvm::dyn_cast<llvm::ConstantInt>(generate(c.second));//create_alloca(function, c.first, llvm::Type::getInt32Ty(m_context));

    auto oldVars = m_variables;
    for (auto& v : m_tree->vars())
        m_variables[v.first] = create_alloca(function, v.first, llvm::Type::getInt32Ty(m_context));
    m_variables[expr->name()] = create_alloca(function, expr->name(), get_type(expr->return_type()));

    // body
    auto block = llvm::BasicBlock::Create(m_context, "entry", function);
    m_builder->SetInsertPoint(block);

    generate(expr->body());

    m_builder->CreateRet(m_variables[expr->name()]);

    m_variables = oldVars;
    m_constants = oldConsts;

    return function;
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
    return function;
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


void CodeGenerator::print() const {
    m_module->print(llvm::errs(), nullptr);
}

llvm::Value *CodeGenerator::generate_code() {
    auto fType = llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), {}, false);
    auto function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "main", m_module.get());
    llvm::BasicBlock* block = llvm::BasicBlock::Create(m_context, "start", function);
    m_builder->SetInsertPoint(block);
    auto extraAlloca = create_alloca(function, "_extra", llvm::Type::getInt8Ty(m_context));
    m_variables["_extra"] = extraAlloca;
    for (auto& c : m_tree->consts())
        m_constants[c.first] = llvm::dyn_cast<llvm::ConstantInt>(generate(c.second));//create_alloca(function, c.first, llvm::Type::getInt32Ty(m_context));

    for (auto& v : m_tree->vars())
        m_variables[v.first] = create_alloca(function, v.first, llvm::Type::getInt32Ty(m_context));
    generate(m_tree->body());
    m_builder->CreateRet(nullptr);
    return function;
}

llvm::Value *CodeGenerator::gen_block(std::shared_ptr<BlockExpression> expr) {
//    auto block = llvm::BasicBlock::Create(m_context, "entry", m_builder->GetInsertBlock()->getParent());
//    m_builder->SetInsertPoint(block);
    for (auto& e : expr->body())
        generate(e);
    return nullptr;
}



//llvm::Value* CodeGenerator::gen_assign(const std::string &name, ExpressionPointer value) {
//    return llvm::Value*(__cxx11::list());
//}


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

    std::system((std::string("clang ") + fileName + " -o " + fileName + ".bin").c_str());
}

void CodeGenerator::add_standard_functions() {
    // printf
    auto printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context),
            {llvm::Type::getInt8PtrTy(m_context)}, true);
    auto printfFun = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage,
            "printf", m_module.get());

    // writeln(int)
    auto writelnType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context),
            {llvm::Type::getInt32Ty(m_context)}, false);
    auto writelnFun = llvm::Function::Create(writelnType, llvm::Function::ExternalLinkage,
            "writeln", m_module.get());
    auto writelnBlk = llvm::BasicBlock::Create(m_context, "start", writelnFun);
    m_builder->SetInsertPoint(writelnBlk);
    llvm::Value *writeStr =  m_builder->CreateGlobalStringPtr("%d\n");
    m_builder->CreateCall(printfType, printfFun, {writeStr, writelnFun->getArg(0)});
    m_builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));

    // scanf
    auto scanfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context),
            {llvm::Type::getInt8PtrTy(m_context)}, true);
    auto scanfFun = llvm::Function::Create(scanfType, llvm::Function::ExternalLinkage,
            "scanf", m_module.get());

    // readln(int)
    auto readlnType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context),
            {llvm::Type::getInt32PtrTy(m_context)}, false);
    auto readlnFun = llvm::Function::Create(readlnType, llvm::Function::ExternalLinkage,
            "readln", m_module.get());
    auto readlnBlk = llvm::BasicBlock::Create(m_context, "start", readlnFun);
    m_builder->SetInsertPoint(readlnBlk);
    //auto ptr = m_builder->CreateIntToPtr(readlnFun->getArg(0), llvm::Type::getInt8PtrTy(m_context));
    //auto ptr = m_builder->CreateLoad(m_builder->CreateIntToPtr(readlnFun->getArg(0), llvm::Type::getInt8PtrTy(m_context)), "ptr");
    llvm::Value* readStr = m_builder->CreateGlobalStringPtr("%d[^\n]");
    m_builder->CreateCall(scanfType, scanfFun, {readStr, readlnFun->getArg(0)});
    m_builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));
}


