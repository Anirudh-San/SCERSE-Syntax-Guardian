#include "Lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace SCERSE {

Lexer::Lexer(const std::string& src)
    : source(src), index(0), currentPosition(1, 1) {
    currentChar = index < source.size() ? source[index] : '\0';
}

void Lexer::advance() {
    if (currentChar == '\n') {
        currentPosition.line++;
        currentPosition.column = 1;
    } else {
        currentPosition.column++;
    }

    index++;
    currentChar = index < source.size() ? source[index] : '\0';
}

void Lexer::skipWhitespace() {
    while (currentChar == ' ' || currentChar == '\t' || currentChar == '\r' || currentChar == '\n') {
        advance();
    }
}

Token Lexer::makeIdentifierOrKeyword() {
    Position startPos = currentPosition;
    std::string lexeme;

    while (std::isalnum(currentChar) || currentChar == '_') {
        lexeme += currentChar;
        advance();
    }

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"var", TokenType::VAR},
        {"int", TokenType::INT},
        {"float", TokenType::FLOAT_KW},
        {"bool", TokenType::BOOL},
        {"string", TokenType::STRING_KW},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"for", TokenType::FOR},
        {"function", TokenType::FUNCTION},
        {"return", TokenType::RETURN},
        {"const", TokenType::CONST},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
        {"void", TokenType::VOID}
    };

    auto it = keywords.find(lexeme);
    if (it != keywords.end())
        return Token(it->second, lexeme, startPos);

    return Token(TokenType::IDENTIFIER, lexeme, startPos);
}

Token Lexer::makeNumber() {
    Position startPos = currentPosition;
    std::string lexeme;
    bool isFloat = false;

    while (std::isdigit(currentChar)) {
        lexeme += currentChar;
        advance();
    }

    if (currentChar == '.') {
        isFloat = true;
        lexeme += currentChar;
        advance();
        while (std::isdigit(currentChar)) {
            lexeme += currentChar;
            advance();
        }
    }

    if (isFloat)
        return Token(TokenType::FLOAT, lexeme, startPos);
    else
        return Token(TokenType::INTEGER, lexeme, startPos);
}

Token Lexer::makeOperatorOrPunctuation() {
    Position pos = currentPosition;
    char ch = currentChar;

    switch (ch) {
        case ';': advance(); return Token(TokenType::SEMICOLON, ";", pos);
        case '(': advance(); return Token(TokenType::LEFT_PAREN, "(", pos);
        case ')': advance(); return Token(TokenType::RIGHT_PAREN, ")", pos);
        case '{': advance(); return Token(TokenType::LEFT_BRACE, "{", pos);
        case '}': advance(); return Token(TokenType::RIGHT_BRACE, "}", pos);
        case '+': advance(); return Token(TokenType::PLUS, "+", pos);
        case '-': advance(); return Token(TokenType::MINUS, "-", pos);
        case '*': advance(); return Token(TokenType::MULTIPLY, "*", pos);
        case '/': advance(); return Token(TokenType::DIVIDE, "/", pos);
        case '%': advance(); return Token(TokenType::MODULO, "%", pos);

        case '=':
            advance();
            if (currentChar == '=') { advance(); return Token(TokenType::EQUAL, "==", pos); }
            return Token(TokenType::ASSIGN, "=", pos);

        case '<':
            advance();
            if (currentChar == '=') { advance(); return Token(TokenType::LESS_EQUAL, "<=", pos); }
            return Token(TokenType::LESS, "<", pos);

        case '>':
            advance();
            if (currentChar == '=') { advance(); return Token(TokenType::GREATER_EQUAL, ">=", pos); }
            return Token(TokenType::GREATER, ">", pos);

        case '!':
            advance();
            if (currentChar == '=') { advance(); return Token(TokenType::NOT_EQUAL, "!=", pos); }
            return Token(TokenType::LOGICAL_NOT, "!", pos);

        case '&':
            advance();
            if (currentChar == '&') { advance(); return Token(TokenType::LOGICAL_AND, "&&", pos); }
            break;

        case '|':
            advance();
            if (currentChar == '|') { advance(); return Token(TokenType::LOGICAL_OR, "||", pos); }
            break;

        case ',': advance(); return Token(TokenType::COMMA, ",", pos);
        case '.': advance(); return Token(TokenType::DOT, ".", pos);
    }

    // Unknown character
    advance();
    return Token(TokenType::ERROR_TOKEN, "Unexpected character: " + std::string(1, ch), pos);
}

Token Lexer::getNextToken() {
    while (isspace(currentChar)) {
        skipWhitespace();
    }

    if (currentChar == '\0')
        return Token(TokenType::EOF_TOKEN, "$", currentPosition);

    if (std::isalpha(currentChar) || currentChar == '_')
        return makeIdentifierOrKeyword();

    if (std::isdigit(currentChar))
        return makeNumber();

    return makeOperatorOrPunctuation();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = getNextToken();

    while (token.type != TokenType::EOF_TOKEN && token.type != TokenType::ERROR_TOKEN) {
        tokens.push_back(token);
        token = getNextToken();
    }

    tokens.push_back(Token(TokenType::EOF_TOKEN, "$", currentPosition));
    return tokens;
}

} // namespace SCERSE
