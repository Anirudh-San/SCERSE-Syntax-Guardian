#include "Lexer.hpp"
namespace SCERSE {

Token Lexer::nextToken() {
    // Placeholder implementation: recognizes identifiers and EOF only
    if (current >= source.length())
        return Token(TokenType::EOF_TOKEN, "", position);

    size_t start = current;
    while (current < source.length() && isalnum(source[current])) {
        current++;
        position.column++;
    }
    std::string text = source.substr(start, current-start);
    if (!text.empty())
        return Token(TokenType::IDENTIFIER, text, position);

    current++;
    position.column++;
    return Token(TokenType::ERROR_TOKEN, std::string(1, source[start]), position);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token t = nextToken();
    while (t.type != TokenType::EOF_TOKEN) {
        tokens.push_back(t);
        t = nextToken();
    }
    tokens.push_back(Token(TokenType::EOF_TOKEN, "", position));
    return tokens;
}

}
