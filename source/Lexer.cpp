//
// Created by askar on 27/04/2020.
//

#include "../include/Lexer.h"

#include "../include/Exception.h"
#include "../include/Syntax.h"

#include <iostream>
#include <memory>

Lexer::Lexer(std::istream &stream) : m_Stream(stream), m_Char(stream.get()), m_Position{1, 0} {}

int Lexer::read_char() {
    m_Char = m_Stream.get();
    if (m_Stream.eof())
        m_Char = std::istream::eofbit;
    else if (m_Char == '\n') {
        ++m_Position.line;
        m_Position.column = 0;
    } else
        ++m_Position.column;
    return m_Char;
}

int Lexer::read_number() {
    int number = m_Char - '0';
    while (std::isdigit(read_char()))
        number = number * 10 + (m_Char - '0');
    return number;
}

std::string Lexer::read_identifier() {
    std::string identifier(1, m_Char);
    while (std::isalpha(read_char()))
        identifier += m_Char;
    return std::move(identifier);
}

// Create a specific token as a pointer to Token base
template<typename T, typename A>
std::shared_ptr<Token> as_token(const A arg) {
    return std::static_pointer_cast<Token>(std::make_shared<T>(arg));
}

std::shared_ptr<Token> Lexer::next_token() {
    while (Syntax::is_delimiter(m_Char))
        read_char();
    switch (m_Char) {
        case std::istream::eofbit:
            return as_token<SimpleToken, TokenType>(TOK_EOF);
        // number
        case '0' ... '1':
            return as_token<TokInteger, int>(read_number());
        // identifier
        case 'a' ... 'z':
        case 'A' ... 'Z': {
            const std::string &&identifier = read_identifier();
            const TokenType type = Syntax::check_keyword(identifier);
            if (!Syntax::check_keyword(identifier))
                return as_token<IdentifierToken, std::string>(identifier);
            return as_token<SimpleToken, TokenType>(type);
        }
        // single char
        default: {
            const TokenType type = Syntax::check_character(m_Char);
            if (type) {
                read_char();
                return as_token<SimpleToken, TokenType>(type);
            }
            throw InvalidSymbolException(m_Position, m_Char);
        }
    }
}

const TextPosition& Lexer::position() { return m_Position; }
