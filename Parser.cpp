//
// Created by askar on 29/04/2020.
//

#include "Parser.h"

Parser::Parser(std::istream &stream) : m_Lexer(stream) {}

void Parser::parse() {
    bool quit = false;
    while (!quit) {
        auto token = m_Lexer.next_token();
        switch(token->type()) {

        }
    }
}


