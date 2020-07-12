//
// Created by askar on 27/04/2020.
//

#include "../include/Lexer.h"

#include "../include/Exception.h"
#include "../include/Syntax.h"

#include <iostream>
#include <memory>

Lexer::Lexer(std::istream &stream) :
    m_stream(stream),
    m_char(stream.get()),
    m_position{1, 1},
    m_prevPosition{1, 1}
    {}

int Lexer::read_char() {
    m_char = m_stream.get();
    if (m_stream.eof())
        m_char = std::istream::eofbit;
    else if (m_char == '\n') {
        ++m_position.line;
        m_position.column = 0;
    } else
        ++m_position.column;
    return m_char;
}

int Lexer::read_number() {
    int number = m_char - '0';
    while (std::isdigit(read_char()))
        number = number * 10 + (m_char - '0');
    return number;
}

std::string Lexer::read_identifier() {
    std::string identifier(1, m_char);
    while (std::isalpha(read_char()))
        identifier += m_char;
    return std::move(identifier);
}

// Create a specific token as a pointer to Token base
template<typename T, typename A>
std::shared_ptr<Token> as_token(const A arg) {
    return std::static_pointer_cast<Token>(std::make_shared<T>(arg));
}

std::shared_ptr<Token> Lexer::next_token() {
    while (Syntax::is_delimiter(m_char))
        read_char();
    m_prevPosition = m_position;
    switch (m_char) {
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
            const TokenType type = Syntax::check_character(m_char);
            if (type) {
                read_char();
                return as_token<SimpleToken, TokenType>(type);
            }
            throw InvalidSymbolException(m_position, m_char);
        }
    }
}

const TextPosition& Lexer::position() { return m_prevPosition; }
