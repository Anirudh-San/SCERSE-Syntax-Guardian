#pragma once

#include "../common/Types.hpp"
#include "../common/Error.hpp"
#include <string>

namespace SCERSE {

// Token.hpp - Complete Token definitions for lexical analysis

/**
 * TokenType enumeration
 * Represents all possible token types in the C language subset
 */
enum class TokenType {
    // Literals
    INTEGER,        // Integer literal: 42, 100
    FLOAT,          // Float literal: 3.14, 2.5
    STRING,         // String literal: "hello"
    BOOLEAN,        // Boolean value (not a keyword, but a parsed boolean)
    IDENTIFIER,     // Variable/function names: x, myVar, func1
    
    // Keywords
    IF,             // if
    ELSE,           // else
    WHILE,          // while
    FOR,            // for
    FUNCTION,       // function
    RETURN,         // return
    VAR,            // var
    CONST,          // const
    TRUE,           // true
    FALSE,          // false
    INT,            // int (type keyword)
    FLOAT_KW,       // float (type keyword)
    STRING_KW,      // string (type keyword)
    BOOL,           // bool (type keyword)
    VOID,           // void (type keyword)
    
    // Arithmetic Operators
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    MODULO,         // %
    
    // Assignment & Comparison
    ASSIGN,         // =
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    LESS,           // <
    LESS_EQUAL,     // <=
    GREATER,        // >
    GREATER_EQUAL,  // >=
    
    // Logical Operators
    LOGICAL_AND,    // &&
    LOGICAL_OR,     // ||
    LOGICAL_NOT,    // !
    
    // Delimiters & Punctuation
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    LEFT_BRACE,     // {
    RIGHT_BRACE,    // }
    LEFT_BRACKET,   // [
    RIGHT_BRACKET,  // ]
    SEMICOLON,      // ;
    COMMA,          // ,
    DOT,            // .
    
    // Special Tokens
    NEWLINE,        // \n (if tracking newlines)
    EOF_TOKEN,      // End of file
    ERROR_TOKEN,    // Error/unknown token
    
    // Aliases
    UNKNOWN = ERROR_TOKEN,
    END_OF_FILE = EOF_TOKEN
};

/**
 * Token class
 * Represents a single lexical token with type, lexeme, and position
 */
class Token {
public:
    TokenType type;         // The type of token
    std::string lexeme;     // The actual text of the token
    Position position;      // Line and column position in source
    
    // Constructor
    Token(TokenType t = TokenType::ERROR_TOKEN, 
          const std::string& lex = "", 
          const Position& pos = Position())
        : type(t), lexeme(lex), position(pos) {}
    
    // Utility Methods
    
    /**
     * Check if token is a keyword
     */
    bool isKeyword() const;
    
    /**
     * Check if token is an operator
     */
    bool isOperator() const;
    
    /**
     * Check if token is a literal (number, string, boolean)
     */
    bool isLiteral() const;
    
    /**
     * Check if token is a delimiter/punctuation
     */
    bool isDelimiter() const;
    
    /**
     * Get string representation of token type
     */
    std::string typeToString() const;
    
    /**
     * Get full string representation of token for debugging
     */
    std::string toString() const;
    
    /**
     * Check if this is an error token
     */
    bool isError() const {
        return type == TokenType::ERROR_TOKEN;
    }
    
    /**
     * Check if this is EOF token
     */
    bool isEOF() const {
        return type == TokenType::EOF_TOKEN;
    }
};

/**
 * Utility function to convert TokenType to string
 */
std::string tokenTypeToString(TokenType type);

/**
 * Utility function to check if a string is a keyword
 */
bool isKeywordString(const std::string& str);

/**
 * Utility function to get TokenType from keyword string
 * Returns TokenType::IDENTIFIER if not a keyword
 */
TokenType keywordStringToTokenType(const std::string& str);

} // namespace SCERSE
