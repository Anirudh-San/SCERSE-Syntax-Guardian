#pragma once
#include <string>
#include <vector>
#include "Token.hpp"

namespace SCERSE {

class Lexer {
private:
    std::string source;
    size_t index;
    char currentChar;
    Position currentPosition;

    void advance();
    void skipWhitespace();
    Token makeIdentifierOrKeyword();
    Token makeNumber();
    Token makeOperatorOrPunctuation();

public:
    Lexer(const std::string& src);
    Token getNextToken();
    std::vector<Token> tokenize();
};

} // namespace SCERSE
