#include "Token.hpp"
#include <sstream>
#include <unordered_map>

namespace SCERSE {

// ========== Token Member Functions ==========

bool Token::isKeyword() const {
    return (type >= TokenType::IF && type <= TokenType::VOID);
}

bool Token::isOperator() const {
    return (type >= TokenType::PLUS && type <= TokenType::LOGICAL_NOT);
}

bool Token::isLiteral() const {
    return (type == TokenType::INTEGER || 
            type == TokenType::FLOAT || 
            type == TokenType::STRING || 
            type == TokenType::BOOLEAN);
}

bool Token::isDelimiter() const {
    return (type >= TokenType::LEFT_PAREN && type <= TokenType::DOT);
}

std::string Token::typeToString() const {
    return tokenTypeToString(type);
}

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "Token(" 
        << typeToString() 
        << ", \"" << lexeme << "\""
        << ", Line:" << position.line 
        << ", Col:" << position.column 
        << ")";
    return oss.str();
}

// ========== Utility Functions ==========

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        // Literals
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::BOOLEAN: return "BOOLEAN";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        
        // Keywords
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::RETURN: return "RETURN";
        case TokenType::VAR: return "VAR";
        case TokenType::CONST: return "CONST";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::INT: return "INT";
        case TokenType::FLOAT_KW: return "FLOAT_KW";
        case TokenType::STRING_KW: return "STRING_KW";
        case TokenType::BOOL: return "BOOL";
        case TokenType::VOID: return "VOID";
        
        // Arithmetic Operators
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::MODULO: return "MODULO";
        
        // Assignment & Comparison
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        
        // Logical Operators
        case TokenType::LOGICAL_AND: return "LOGICAL_AND";
        case TokenType::LOGICAL_OR: return "LOGICAL_OR";
        case TokenType::LOGICAL_NOT: return "LOGICAL_NOT";
        
        // Delimiters & Punctuation
        case TokenType::LEFT_PAREN: return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        
        // Special Tokens
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::EOF_TOKEN: return "EOF_TOKEN";
        case TokenType::ERROR_TOKEN: return "ERROR_TOKEN";
        
        default: return "UNKNOWN";
    }
}

bool isKeywordString(const std::string& str) {
    static const std::unordered_map<std::string, bool> keywords = {
        {"var", true}, {"int", true}, {"float", true}, {"bool", true}, 
        {"string", true}, {"if", true}, {"else", true}, {"while", true}, 
        {"for", true}, {"function", true}, {"return", true}, {"const", true}, 
        {"true", true}, {"false", true}, {"void", true}
    };
    return keywords.find(str) != keywords.end();
}

TokenType keywordStringToTokenType(const std::string& str) {
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
    
    auto it = keywords.find(str);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}

} // namespace SCERSE
