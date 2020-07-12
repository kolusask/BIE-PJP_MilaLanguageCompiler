//
// Created by askar on 27/04/2020.
//

#ifndef MILALANGUAGECOMPILER_LEXER_H
#define MILALANGUAGECOMPILER_LEXER_H

#include "Token.h"

#include <istream>
#include <map>
#include <memory>
#include <set>

struct TextPosition {
    size_t line, column;
};

class Lexer {
public:
    Lexer(std::istream& stream);
    std::shared_ptr<Token> next_token();
    const TextPosition& position();

private:
    int read_char();
    int read_number();
    std::string read_identifier();
    std::istream& m_stream;
    int m_char;
    TextPosition m_position;
    TextPosition m_prevPosition;

};


#endif //MILALANGUAGECOMPILER_LEXER_H
