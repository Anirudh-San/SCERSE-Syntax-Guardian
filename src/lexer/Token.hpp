#pragma once
#include "../common/Types.hpp"
#include "../common/Error.hpp"
#include <string>
namespace SCERSE {

enum class TokenType {
    INTEGER, FLOAT, STRING, BOOLEAN, IDENTIFIER,
    IF, ELSE, WHILE, FOR, FUNCTION, RETURN, VAR, CONST, TRUE, FALSE, INT, FLOAT_KW, STRING_KW, BOOL, VOID,
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, ASSIGN, EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    LOGICAL_AND, LOGICAL_OR, LOGICAL_NOT,
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, LEFT_BRACKET, RIGHT_BRACKET, SEMICOLON, COMMA, DOT,
    NEWLINE, EOF_TOKEN, ERROR_TOKEN,
    UNKNOWN = ERROR_TOKEN,
    END_OF_FILE = EOF_TOKEN,
    PROGRAM, STATEMENT_LIST, STATEMENT, EXPRESSION, TERM, FACTOR
};


class Token {
public:
    TokenType type;
    std::string lexeme;
    Position position;
    Token(TokenType t = TokenType::ERROR_TOKEN, const std::string& lex = "", const Position& pos = Position())
        : type(t), lexeme(lex), position(pos) {}
};

}
