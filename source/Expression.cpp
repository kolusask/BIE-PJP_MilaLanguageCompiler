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




//////////////////////////////////////////////////
//  Debug outputs
//////////////////////////////////////////////////
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

std::string ParenthesesExpression::to_string() const {
    return '(' + m_expression->to_string() + ')';
}

std::string BinaryOperationExpression::to_string() const {
    return '(' + m_left->to_string() + ')' + m_operator->to_string() + '(' + m_right->to_string() + ')';
}

std::string ConstExpression::to_string() const {
    std::ostringstream oss;
    for (const auto& c : m_consts)
        oss << "const " << c.first << '=' << c.second->to_string() << ';' << std::endl;
    return oss.str();
}

std::string VarExpression::to_string() const {
    std::ostringstream oss;
    for (const auto& v : m_vars)
        oss << "var " << v.first << " : " << "integer" << ';' << std::endl;
    return oss.str();
}

std::string FunctionExpression::to_string() const {
    std::ostringstream oss("function ", std::ios::ate);
    oss << m_name << '(';
    bool first = true;
    for (const Variable& arg : m_arguments) {
        if (first)
            first = false;
        else
            oss << "; ";
        oss << arg.first << ": " << arg.second;
    }
    static const std::map<TokenType, std::string> dataTypeMap = {{TOK_INTEGER, "integer"}};
    oss << "): " << dataTypeMap.at(m_type) << ';' << std::endl;
    if (m_consts)
        oss << m_consts->to_string();
    if (m_vars)
        oss << m_vars->to_string();
    oss << m_body->to_string() << ';' << std::endl;
    return oss.str();
}

std::string TopLevelExpression::to_string() const {
    std::ostringstream oss;
    if (m_consts)
        oss << m_consts->to_string();
    if (m_vars)
        oss << m_vars->to_string();
    for (const auto& fun : m_functions)
        oss << fun->to_string();
    oss << m_body->to_string() << '.';
    return oss.str();
}

std::string ConditionExpression::to_string() const {
    std::ostringstream oss;
    oss << "if " << m_condition->to_string() << " then" << std::endl;
    oss << m_ifTrue->to_string() << std::endl;
    if (m_ifFalse)
        oss << "else " << std::endl << m_ifFalse->to_string();
    return oss.str();
}

std::string WhileLoopExpression::to_string() const {
    std::stringstream oss;
    oss << "while " << m_condition->to_string() << " do" << std::endl;
    oss << m_body->to_string();
    return oss.str();
}


std::string ForLoopExpression::to_string() const {
    std::stringstream oss;
    oss << "for " << m_counter << " := "
        << m_start->to_string() << (m_down ? " downto " : " to ")
        << m_finish->to_string() << " do " << std::endl;
    oss << m_body->to_string();
    return oss.str();
}
