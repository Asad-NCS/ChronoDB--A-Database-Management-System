#include "lexer.h"
#include <cctype>
#include <unordered_set>

namespace ChronoDB {

    Lexer::Lexer(std::string input) : src(std::move(input)) {}

    char Lexer::current() {
        if (pos >= src.size()) return '\0';
        return src[pos];
    }

    void Lexer::advance() { pos++; }

    void Lexer::skipWhitespace() {
        while (isspace(current())) advance();
    }

    Token Lexer::readString() {
        advance(); // Skip opening quote
        std::string val;
        while (current() != '"' && current() != '\0') {
            val += current();
            advance();
        }
        advance(); // Skip closing quote
        return {TokenType::STRING_LITERAL, val};
    }

    Token Lexer::readNumber() {
        std::string val;
        while (isdigit(current()) || current() == '.') {
            val += current();
            advance();
        }
        return {TokenType::NUMBER, val};
    }

    Token Lexer::readIdentifierOrKeyword() {
        std::string val;
        while (isalnum(current()) || current() == '_') {
            val += current();
            advance();
        }
        // Simple check for keywords (could be expanded)
        return {TokenType::IDENTIFIER, val}; // Parser will decide if it's a keyword
    }

    Token Lexer::nextToken() {
        skipWhitespace();
        if (current() == '\0') return {TokenType::END_OF_FILE, ""};

        if (isalpha(current())) return readIdentifierOrKeyword();
        if (isdigit(current())) return readNumber();
        if (current() == '"') return readString();

        // Handle multi-character symbols
        char c = current();
        advance();
        
        // Check for two-character operators
        if ((c == '=' || c == '!' || c == '<' || c == '>') && current() == '=') {
            std::string sym(1, c);
            sym += '=';
            advance();
            return {TokenType::SYMBOL, sym};
        }
        
        // Single char symbols
        std::string sym(1, c);
        return {TokenType::SYMBOL, sym};
    }

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        Token t = nextToken();
        while (t.type != TokenType::END_OF_FILE) {
            tokens.push_back(t);
            t = nextToken();
        }
        return tokens;
    }
}