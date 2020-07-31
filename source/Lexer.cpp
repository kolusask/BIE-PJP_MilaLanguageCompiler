//
// Created by askar on 27/04/2020.
//

#include "../include/Lexer.h"

#include "../include/Exception.h"
#include "../include/Syntax.h"

#include <iostream>
#include <memory>
#include <sstream>

Lexer::Lexer(std::istream &stream) :
    m_stream(stream),
    m_char(stream.get()),
    m_position{1, 1},
    m_prevPosition{1, 1},
    m_lastToken(nullptr)
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

double Lexer::read_number(bool& isDouble) {
    std::string number(1, m_char);
    while (std::isdigit(read_char()) || (!isDouble && (isDouble = (m_char == '.'))))
        number += m_char;
    return std::stod(number);
}

int Lexer::read_hex() {
    std::stringstream reading;
    while (std::isdigit(read_char()))
        reading << m_char;
    int result;
    std::cout << reading.str() << std::endl;
    reading >> std::hex >> result;
    return result;
}

int Lexer::read_oct() {
    std::stringstream reading;
    while (std::isdigit(read_char()))
        reading << m_char;
    int result;
    reading >> std::oct >> result;
    return result;
}

std::string Lexer::read_identifier() {
    std::string identifier(1, m_char);
    while (std::isalpha(read_char()) || std::isdigit(m_char) || m_char == '_')
        identifier += m_char;
    return std::move(identifier);
}

std::string Lexer::read_operator() {
    if (m_char == '=') {
        read_char();
        return "=";
    }
    std::string op(1, m_char);
    while (is_in_operator(read_char())) {
        op += m_char;
        if (m_char == '=') {
            read_char();
            break;
        }
    }
    return std::move(op);
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
        case '0' ... '9': {
            bool isDouble = false;
            double value = read_number(isDouble);
            if (isDouble)
                return as_token<DoubleToken, double>(value);
            return as_token<IntegerToken, int>(int(value));
        }
        // multi character string
        case 'a' ... 'z':
        case 'A' ... 'Z': {
            const std::string &&identifier = read_identifier();
            TokenType type;
            if ((type = Syntax::check_keyword(identifier)))
                return as_token<SimpleToken, TokenType>(type);
            if ((type = Syntax::check_operator(identifier)))
                return as_token<OperatorToken, TokenType>(type);
            return as_token<IdentifierToken, std::string>(identifier);

        }
        // operator
        case '<':
        case '=':
        case '>':
        case '+':
        case '-':
        case '*':
        case '/':
        case ':': {
            const std::string &&op = read_operator();
            TokenType type;
            if ((type = Syntax::check_operator(op)))
                return as_token<OperatorToken, TokenType>(type);
            if (op.length() == 1 && (type = Syntax::check_character(op[0])))
                return as_token<SimpleToken, TokenType>(type);
            throw InvalidSymbolException(m_position, m_char);
        }
        case '$':   // hex
            return as_token<IntegerToken, int>(read_hex());
        case '&':
            return as_token<IntegerToken, int>(read_oct());
        // string
        case '"':
            return as_token<StringToken, std::string>(read_string());
        //single char
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

bool Lexer::is_in_operator(const char ch) const {
    static const std::set<char> op_set = {'+',
                                          '-',
                                          '*',
                                          '/',
                                          '<',
                                          '=',
                                          '>',
                                          ':'};
    return op_set.count(ch);
}

std::string Lexer::read_string() {
    std::ostringstream result;
    bool escape = false;
    while (read_char() != '"' || escape) {
        if (escape) {
            switch (m_char) {
                case '\\':
                case '\"':
                    result << m_char;
                    break;
                case 'n':
                    result << '\n';
                    break;
                default:
                    throw InvalidSymbolException(m_position, m_char);
            }
            escape = false;
        } else {
            if (m_char == '\\')
                escape = true;
            else
                result << m_char;
        }
    }
    read_char();
    return result.str();
}


