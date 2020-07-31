//
// Created by askar on 08/05/2020.
//

#include "../include/Token.h"

#include "../include/Operators.h"


int OperatorToken::op_precedence() const {
    static std::map<TokenType, int> precMap;
    if (precMap.empty())
        for (const auto& op : g_operators)
            precMap[op.type] = op.precedence;
    auto it = precMap.find(m_type);
    if (it == precMap.end())
        return -1;
    return it->second;
}

std::string OperatorToken::to_string() const {
    static std::map<TokenType, const char*> opStrings;
    if (opStrings.empty())
        for (const auto& op : g_operators)
            opStrings[op.type] = op.name.c_str();
    auto it = opStrings.find(m_type);
    if (it == opStrings.end())
        return "<?>";
    return it->second;
}
