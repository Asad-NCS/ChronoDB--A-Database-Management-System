//Tokenize input commands
#ifndef CHRONODB_LEXER_H
#define CHRONODB_LEXER_H

#include <string>
#include <vector>
#include "../utils/helpers.h"

namespace ChronoDB {

    enum class TokenType {
        KEYWORD, IDENTIFIER, STRING_LITERAL, NUMBER, SYMBOL, END_OF_FILE
    };

    struct Token {
        TokenType type;
        std::string value;
    };

    class Lexer {
    public:
        explicit Lexer(std::string input);
        Token nextToken();
        std::vector<Token> tokenize(); // Returns all tokens at once

    private:
        std::string src;
        size_t pos = 0;
        char current();
        void advance();
        void skipWhitespace();
        Token readIdentifierOrKeyword();
        Token readNumber();
        Token readString();
    };

}

#endif