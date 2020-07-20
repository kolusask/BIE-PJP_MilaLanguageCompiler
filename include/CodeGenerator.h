//
// Created by askar on 20/07/2020.
//

#ifndef BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H
#define BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H

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

private:


    llvm::LLVMContext m_context;
    llvm::IRBuilder<> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    std::map<std::string, llvm::Value *> m_namedValues;
};


#endif //BIE_PJP_MILALANGUAGECOMPILER_CODEGENERATOR_H
