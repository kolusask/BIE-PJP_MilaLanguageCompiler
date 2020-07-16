//
// Created by askar on 12/07/2020.
//

#include "../include/Expression.h"

llvm::LLVMContext Expression::s_context;
llvm::IRBuilder<> Expression::s_builder(Expression::s_context);
std::unique_ptr<llvm::Module> Expression::s_module =
        std::make_unique<llvm::Module>("JIT Module", Expression::s_context);
std::map<std::string, llvm::Value *> Expression::s_namedValues;

/////////////////////////////////////////////////
//  Code generation
/////////////////////////////////////////////////
llvm::Value* IntegerExpression::codegen() const {
    return llvm::ConstantInt::get(s_context, llvm::APSInt(m_value));
}




/////////////////////////////////////////////////////////
//  Debug outputs
/////////////////////////////////////////////////////////
std::string InitializationExpression::to_string() const {
    return m_key + '=' + m_value->to_string();
}

std::string LocalsExpression::to_string() const {
    std::ostringstream oss(keyword(), std::ios::ate);
    for (const auto& exp : m_initializations)
    oss << exp->to_string() << ';' << std::endl;
    return oss.str();
}

std::string IntegerExpression::to_string() const {
    return std::to_string(m_value);
}

std::string IdentifierExpression::to_string() const {
    return m_value;
}

std::string CallExpression::to_string() const {
    std::ostringstream oss(m_name + '(', std::ios::ate);
    bool first = true;
    for (const auto& arg : m_arguments) {
        if (!first)
            oss << ',';
        oss << arg->to_string();
        first = false;
    }
    return oss.str() + ')';
}


std::string BlockExpression::to_string() const {
    std::ostringstream oss("begin\n", std::ios::ate);
    for (const auto& expr : m_body)
        oss << expr->to_string() << ";" << std::endl;
    oss << "end";
    return oss.str();
}

std::string BodyExpression::to_string() const {
    std::ostringstream oss;
    if (m_const)
        oss << m_const->to_string() << std::endl;
    if (m_var)
        oss << m_var->to_string();
    if (m_block)
        oss << m_block->to_string();
    return oss.str();
}

std::string ParenthesesExpression::to_string() const {
    return '(' + m_expression->to_string() + ')';
}

std::string BinaryOperationExpression::to_string() const {
    return '(' + m_left->to_string() + ')' + m_operator->to_string() + '(' + m_right->to_string() + ')';
}
