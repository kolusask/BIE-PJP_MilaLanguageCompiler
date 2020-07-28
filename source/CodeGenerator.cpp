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

llvm::Value * CodeGenerator::generate(const ExpressionPointer expr, llvm::BasicBlock *breakTo = nullptr,
                                      llvm::BasicBlock *exitTo=nullptr) {
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
            return std::move(gen_condition(std::move(std::static_pointer_cast<ConditionExpression>(expr)), breakTo,
                                           exitTo));
        case EXPR_ASSIGN:
            return std::move(gen_assign(std::move(std::static_pointer_cast<AssignExpression>(expr))));
        case EXPR_WHILE_LOOP:
            return std::move(gen_while(std::move(std::static_pointer_cast<WhileLoopExpression>(expr)), exitTo));
        case EXPR_BREAK:
            return std::move(gen_break(breakTo, expr->position()));
        case EXPR_BLOCK:
            return std::move(gen_block(std::static_pointer_cast<BlockExpression>(expr), breakTo, exitTo));
        case EXPR_FOR_LOOP:
            return std::move(gen_for(std::static_pointer_cast<ForLoopExpression>(expr), exitTo));
        case EXPR_EXIT:
            return std::move(gen_exit(exitTo, expr->position()));
        case EXPR_PARENTHESES:
            return std::move(gen_parentheses(std::static_pointer_cast<ParenthesesExpression>(expr)));
        case EXPR_DOUBLE:
            return std::move(gen_double(std::static_pointer_cast<DoubleExpression>(expr)));
        default:
            throw Exception("NOT IMPLEMENTED");
    }
}

llvm::Value* CodeGenerator::gen_integer(const std::shared_ptr<IntegerExpression> expr) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), expr->value());
}

llvm::Value* CodeGenerator::gen_identifier(const std::shared_ptr<IdentifierExpression> expr) {
    llvm::Value* value;
    if ((value = m_constants[expr->value()]))
        return value;
    if ((value = m_variables[expr->value()]) || ((value = m_globals[expr->value()])))
        return m_builder->CreateLoad(value, expr->value().c_str());

    throw Exception(expr->position(), "Unknown identifier '" + expr->value() + '\'');
}

llvm::Value * CodeGenerator::to_double(llvm::Value *value, llvm::Type *type) {
    auto origType = value->getType();
    if (!type)
        type = llvm::Type::getDoubleTy(m_context);
    return m_builder->CreateSIToFP(value, type, "double");
}

llvm::Value* CodeGenerator::gen_binary_operation(const std::shared_ptr<BinaryOperationExpression> ep) {
    auto expr = std::static_pointer_cast<BinaryOperationExpression>(ep);

    auto left = generate(expr->left(), nullptr, nullptr);
    auto right = generate(expr->right(), nullptr, nullptr);

    if (left->getType() == m_builder->getDoubleTy() || right->getType() == m_builder->getDoubleTy()) {
        if (left->getType() == m_builder->getInt32Ty())
            left = m_builder->CreateSIToFP(left, m_builder->getDoubleTy(), "doubleleft");
        else if (right->getType() == m_builder->getInt32Ty())
            right = m_builder->CreateSIToFP(right, m_builder->getDoubleTy(), "doubleright");
        switch (expr->op()->type()) {
            case TOK_PLUS:
                return m_builder->CreateFAdd(left, right, "addtmp");
            case TOK_MINUS:
                return m_builder->CreateFSub(left, right, "subtmp");
            case TOK_MULTIPLY:
                return m_builder->CreateFMul(left, right, "multmp");
            case TOK_LESS:
                return m_builder->CreateFCmpOLT(left, right, "cmptmp");
            case TOK_LESS_OR_EQUAL:
                return m_builder->CreateFCmpOLE(left, right, "cmptmp");
            case TOK_GREATER:
                return m_builder->CreateFCmpOGT(left, right, "cmptmp");
            case TOK_GREATER_OR_EQUAL:
                return m_builder->CreateFCmpOGE(left, right, "cmptmp");
            case TOK_MOD:
                return m_builder->CreateFRem(left, right, "cmptmp");
            case TOK_AND:
                return m_builder->CreateAnd(left, right, "cmptmp");
            case TOK_OR:
                return m_builder->CreateOr(left, right, "cmptmp");
            case TOK_NOT_EQUAL:
                return m_builder->CreateFCmpONE(left, right, "cmptmp");
            case TOK_EQUAL:
                return m_builder->CreateFCmpOEQ(left, right, "cmptmp");
            default: throw Exception(expr->position(), "NOT IMPLEMENTED");
        }
    }

    switch (expr->op()->type()) {
        case TOK_PLUS:
            return m_builder->CreateAdd(left, right, "addtmp");
        case TOK_MINUS:
            return m_builder->CreateSub(left, right, "subtmp");
        case TOK_MULTIPLY:
            return m_builder->CreateMul(left, right, "multmp");
        case TOK_LESS:
            return m_builder->CreateICmpSLT(left, right, "cmptmp");
        case TOK_LESS_OR_EQUAL:
            return m_builder->CreateICmpSLE(left, right, "cmptmp");
        case TOK_GREATER:
            return m_builder->CreateICmpSGT(left, right, "cmptmp");
        case TOK_GREATER_OR_EQUAL:
            return m_builder->CreateICmpSGE(left, right, "cmptmp");
        case TOK_MOD:
            return m_builder->CreateSRem(left, right, "cmptmp");
        case TOK_AND:
            return m_builder->CreateAnd(left, right, "cmptmp");
        case TOK_OR:
            return m_builder->CreateOr(left, right, "cmptmp");
        case TOK_NOT_EQUAL:
            return m_builder->CreateICmpNE(left, right, "cmptmp");
        case TOK_EQUAL:
            return m_builder->CreateICmpEQ(left, right, "cmptmp");
        default: throw Exception(expr->position(), "NOT IMPLEMENTED");
    }

}

llvm::Value* CodeGenerator::gen_call(const std::shared_ptr<CallExpression> ep) {
    auto expr = std::static_pointer_cast<CallExpression>(ep);

    auto function = m_module->getFunction(expr->name());
    if (expr->name() == "writeln" && expr->args().size() == 1) {
        auto arg = generate(*expr->args().cbegin());
        if (arg->getType() == get_type(TOK_INTEGER))
            function = m_module->getFunction("writeInt");
        else if (arg->getType() == get_type(TOK_DOUBLE))
            function = m_module->getFunction("writeDouble");
    }
    if (!function)
        throw Exception(expr->position(), "Function is not defined: " + expr->name());

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
            llvm::Value* value;
            if ((value = m_variables[ident->value()]) || (value = m_globals[ident->value()]))
                args.push_back(value);
            else {
                if (m_constants[ident->value()])
                    throw Exception(arg->position(), "Cannot read to constant");
                throw Exception(arg->position(), "Unknown identifier: " + ident->value());
            }
        } else
            throw Exception(arg->position(), "Can only read into a variable");
    } else {
        for (const auto &arg : expr->args())
            args.push_back(generate(arg, nullptr, nullptr));
    }
    auto call = m_builder->CreateCall(function, args,
            function->getReturnType() == m_builder->getVoidTy() ? "" : "calltmp");
    if (expr->name() == "readln")
        assign("_extra", m_builder->getInt32(0), expr->position());
    return call;
}

llvm::Type * CodeGenerator::get_type(TokenType type) {
    switch (type) {
        case TOK_INTEGER:
            return llvm::Type::getInt32Ty(m_context);
        case TOK_DOUBLE:
            return llvm::Type::getDoubleTy(m_context);
        case TOK_VOID:
            return llvm::Type::getVoidTy(m_context);
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
    // Function type
    std::vector<llvm::Type*> argTypes;
    for (auto& tt : expr->arg_types())
        argTypes.push_back(get_type(tt));
    auto retType = get_type(expr->return_type());
    auto functionType = llvm::FunctionType::get(retType, argTypes, false);

    auto function = llvm::Function::Create(
            functionType, llvm::Function::ExternalLinkage, expr->name(), m_module.get());
    auto body = llvm::BasicBlock::Create(m_context, "entry", function);
    m_builder->SetInsertPoint(body);
    auto argNames = expr->arg_names();
    size_t i = 0;
    for (auto& arg : function->args())
        arg.setName(argNames[i++]);
    m_variables.clear();
    for (auto& arg : function->args()) {
        auto alloca = create_alloca(function, arg.getName(), arg.getType());
        m_builder->CreateStore(&arg, alloca);
        m_variables[std::string(arg.getName())] = alloca;
    }

    // Vars and consts
    auto oldConsts = m_constants;
    for (auto& c : expr->consts())
        m_constants[c.first] = llvm::dyn_cast<llvm::ConstantInt>(generate(c.second, nullptr, nullptr));//create_alloca(function, c.first, llvm::Type::getInt32Ty(m_context));

    auto oldVars = m_variables;
    for (auto& v : expr->vars())
        m_variables[v.first] = create_alloca(function, v.first, get_type(v.second));
    if (expr->return_type() != TOK_VOID)
        m_variables[expr->name()] = create_alloca(function, expr->name(), get_type(expr->return_type()));


    // body
    auto retBlock = llvm::BasicBlock::Create(m_context, "return", function);
    m_builder->SetInsertPoint(body);

    generate(expr->body(), nullptr, retBlock);
    m_builder->CreateBr(retBlock);
    m_builder->SetInsertPoint(retBlock);

    auto retVal = expr->return_type() == TOK_VOID ? nullptr : m_builder->CreateLoad(m_variables[expr->name()]);

    m_builder->CreateRet(retVal);

    m_constants = oldConsts;

    return function;
}

llvm::AllocaInst *CodeGenerator::create_alloca(llvm::Function *function, const std::string &name, llvm::Type *type) {
    llvm::IRBuilder<> builder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return builder.CreateAlloca(type, 0, name.c_str());
}

llvm::Value *CodeGenerator::gen_while(const std::shared_ptr<WhileLoopExpression> expr, llvm::BasicBlock *exitTo) {
    auto condValue = generate(expr->condition(), nullptr, nullptr);

    auto function = m_builder->GetInsertBlock()->getParent();
    auto goBlock = llvm::BasicBlock::Create(m_context, "go", function);
    auto afterBlock = llvm::BasicBlock::Create(m_context, "after");

    m_builder->CreateCondBr(condValue, goBlock, afterBlock);

    m_builder->SetInsertPoint(goBlock);

    generate(expr->body(), afterBlock, exitTo);
    condValue = generate(expr->condition(), nullptr, nullptr);
    m_builder->CreateCondBr(condValue, goBlock, afterBlock);
    function->getBasicBlockList().push_back(afterBlock);
    m_builder->SetInsertPoint(afterBlock);
    return function;
}

llvm::Value * CodeGenerator::gen_condition(const std::shared_ptr<ConditionExpression> ep, llvm::BasicBlock *breakTo,
                                           llvm::BasicBlock *exitTo) {
    auto expr = std::static_pointer_cast<ConditionExpression>(ep);
    // if-condition
    auto condValue = generate(expr->condition(), nullptr, nullptr);

    auto function = m_builder->GetInsertBlock()->getParent();
    // blocks
    auto thenBlock = llvm::BasicBlock::Create(m_context, "then", function);
    auto elseBlock = llvm::BasicBlock::Create(m_context, "else");
    auto mergeBlock = llvm::BasicBlock::Create(m_context, "ifcont");
    // split
    m_builder->CreateCondBr(condValue, thenBlock, elseBlock);
    // then
    m_builder->SetInsertPoint(thenBlock);
    generate(expr->thenBody(), breakTo, exitTo);
    m_builder->CreateBr(mergeBlock);
    thenBlock = m_builder->GetInsertBlock();
    // else
    function->getBasicBlockList().push_back(elseBlock);
    m_builder->SetInsertPoint(elseBlock);
    if (expr->elseBody())
        generate(expr->elseBody(), breakTo, exitTo);
    m_builder->CreateBr(mergeBlock);
    // merge
    function->getBasicBlockList().push_back(mergeBlock);
    m_builder->SetInsertPoint(mergeBlock);
    return function;
}

llvm::Value* CodeGenerator::gen_assign(const std::shared_ptr<AssignExpression> expr) {
    return assign(std::move(expr->name()), generate(std::move(expr->value()), nullptr, nullptr), expr->position());
}


void CodeGenerator::print() const {
    m_module->print(llvm::errs(), nullptr);
}

llvm::Value *CodeGenerator::generate_code() {

    auto zero = m_builder->getInt32(0);
    for (auto& c : m_tree->consts())
        m_constants[c.first] = llvm::dyn_cast<llvm::Constant>(generate(c.second, nullptr, nullptr));
    for (auto& v : m_tree->vars()) {
        auto global = new llvm::GlobalVariable(*m_module, get_type(v.second), false,
                                               llvm::GlobalVariable::ExternalLinkage, zero, v.first);
        m_globals[v.first] = global;
    }
    auto global = new llvm::GlobalVariable(*m_module, get_type(TOK_INTEGER), false,
                                           llvm::GlobalVariable::ExternalLinkage, zero, "_extra");
    m_globals["_extra"] = global;

    for (const auto& fun : m_tree->functions())
        gen_function(fun);

    auto fType = llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), {}, false);
    auto function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "main", m_module.get());
    auto body = llvm::BasicBlock::Create(m_context, "start", function);
    m_builder->SetInsertPoint(body);
    auto exitBlock = llvm::BasicBlock::Create(m_context, "exit", function);
    gen_block(m_tree->body(), nullptr, exitBlock);
    m_builder->CreateBr(exitBlock);
    m_builder->SetInsertPoint(exitBlock);
    m_builder->CreateRet(nullptr);
    return function;
}

llvm::Value *CodeGenerator::gen_block(const std::shared_ptr<BlockExpression> expr, llvm::BasicBlock *breakTo,
                                      llvm::BasicBlock *exitTo) {
    for (auto& e : expr->body())
        generate(e, breakTo, exitTo);
    return nullptr;
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

    std::system((std::string("clang ") + fileName + " -o " + fileName + ".bin").c_str());
}

void CodeGenerator::add_standard_functions() {
    // printf
    auto printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context),
            {llvm::Type::getInt8PtrTy(m_context)}, true);
    auto printfFun = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage,
            "printf", m_module.get());

    // writeln(int)
    auto writeIntType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context),
            {llvm::Type::getInt32Ty(m_context)}, false);
    auto writeIntFun = llvm::Function::Create(writeIntType, llvm::Function::ExternalLinkage,
            "writeInt", m_module.get());
    auto writeIntBlk = llvm::BasicBlock::Create(m_context, "start", writeIntFun);
    m_builder->SetInsertPoint(writeIntBlk);
    llvm::Value *writeIntStr =  m_builder->CreateGlobalStringPtr("%d\n");
    m_builder->CreateCall(printfType, printfFun, {writeIntStr, writeIntFun->getArg(0)});
    m_builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 0));

    // writeln(double)
    auto writeDoubleType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context),
            {llvm::Type::getDoubleTy(m_context)}, false);
    auto writeDoubleFun = llvm::Function::Create(writeDoubleType, llvm::Function::ExternalLinkage,
            "writeDouble", m_module.get());
    auto writeDoubleBlk = llvm::BasicBlock::Create(m_context, "start", writeDoubleFun);
    m_builder->SetInsertPoint(writeDoubleBlk);
    llvm::Value *writeDoubleStr = m_builder->CreateGlobalStringPtr("%lf\n");
    m_builder->CreateCall(printfType, printfFun, {writeDoubleStr, writeDoubleFun->getArg(0)});
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

llvm::Value *CodeGenerator::gen_break(llvm::BasicBlock *breakTo, TextPosition position) {
    if (breakTo)
        return m_builder->CreateBr(breakTo);
    throw Exception(position, "Break statement outside of loop");
}

llvm::Value *CodeGenerator::gen_for(const std::shared_ptr<ForLoopExpression> expr, llvm::BasicBlock *exitTo) {
    auto function = m_builder->GetInsertBlock()->getParent();
    auto controlBlock = llvm::BasicBlock::Create(m_context, "control", function);
    auto bodyBlock = llvm::BasicBlock::Create(m_context, "for_body", function);
    auto afterBlock = llvm::BasicBlock::Create(m_context, "after", function);

    auto start = generate(expr->start(), nullptr, nullptr);
    auto finish = generate(expr->finish(), nullptr, nullptr);
    assign(expr->counter(), start, expr->position());
    m_builder->CreateBr(controlBlock);

    m_builder->SetInsertPoint(controlBlock);
    auto countValue = load(expr->counter(), expr->position());
    if (countValue->getType() != llvm::Type::getInt32Ty(m_context))
        throw Exception(expr->position(), "For-loop counter must be an integer");
    auto stop = m_builder->CreateICmpEQ(countValue, finish, "stop");
    m_builder->CreateCondBr(stop, afterBlock, bodyBlock);

    m_builder->SetInsertPoint(bodyBlock);
    generate(expr->body(), afterBlock, exitTo);
    static auto one = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), 1);
    llvm::Value* newCount;
    if (expr->down())
        newCount = m_builder->CreateSub(countValue, one, "newcount");
    else
        newCount = m_builder->CreateAdd(countValue, one, "newcount");
    assign(std::move(expr->counter()), newCount, expr->position());
    m_builder->CreateBr(controlBlock);

    m_builder->SetInsertPoint(afterBlock);
    return function;
}

llvm::Value *CodeGenerator::assign(std::string name, llvm::Value *value, TextPosition position) {
    llvm::Value* var;
    if ((var = m_variables[name]) || (var = m_globals[name]))
        return m_builder->CreateStore(value, var);
    if (m_constants[name])
        throw Exception(std::move(position), "Cannot change constant: " + name);
    throw Exception(std::move(position), "Unknown identifier: " + name);
}

llvm::Value *CodeGenerator::load(const std::string &name, TextPosition position) {
    llvm::Value* value;
    if ((value = m_constants[name]))
        return value;
    if ((value = m_variables[name]) || (value = m_globals[name]))
        return m_builder->CreateLoad(value, name);
    throw Exception(std::move(position), "Unknown identifier: " + name);
}

llvm::Value *CodeGenerator::gen_exit(llvm::BasicBlock *exitTo, TextPosition position) {
    if (exitTo)
        return m_builder->CreateBr(exitTo);
    throw Exception("");
}

llvm::Value *CodeGenerator::gen_parentheses(std::shared_ptr<ParenthesesExpression> expr) {
    return generate(expr->expression());
}

llvm::Value *CodeGenerator::gen_double(std::shared_ptr<DoubleExpression> expr) {
    auto val = llvm::ConstantFP::get(m_builder->getDoubleTy(), expr->value());
    return val;
}



