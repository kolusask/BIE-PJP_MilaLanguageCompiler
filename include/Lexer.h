//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_LEXER_H
#define MILALANGUAGECOMPILER_LEXER_H

#include "TextPosition.h"
#include "Token.h"

#include <istream>
#include <map>
#include <memory>
#include <set>

class Lexer {
public:
    Lexer(std::istream& stream);
    std::shared_ptr<Token> next_token();
    const TextPosition& position();

private:
    int read_char();
    double read_number(bool& isDouble);
    int read_hex();
    int read_oct();
    std::string read_identifier();
    std::string read_operator();
    bool is_in_operator(const char ch) const;
    std::istream& m_stream;
    char m_char;
    TextPosition m_position;
    TextPosition m_prevPosition;
    std::shared_ptr<Token> m_lastToken;

};


#endif //MILALANGUAGECOMPILER_LEXER_H
