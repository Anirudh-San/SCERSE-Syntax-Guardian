#pragma once
#include <memory>
#include <vector>
#include <string>
#include "Types.hpp"

namespace SCERSE {

// Define SourcePosition if not already defined elsewhere
struct SourcePosition {
    int line;
    int column;
    
    SourcePosition() : line(0), column(0) {}
    SourcePosition(int l, int c) : line(l), column(c) {}
};

enum class ASTNodeType {
    // Program structure
    PROGRAM,
    STATEMENT_LIST,
    STATEMENT,
    EXPRESSION,
    TERM,
    FACTOR,
    
    // Declarations
    VARIABLE_DECLARATION,
    FUNCTION_DECLARATION,
    PARAMETER_LIST,
    TYPE_SPECIFIER,
    
    // Statements
    BLOCK_STATEMENT,
    EXPRESSION_STATEMENT,
    RETURN_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    
    // Expressions
    ASSIGNMENT,
    BINARY_OPERATION,
    UNARY_OPERATION,
    FUNCTION_CALL,
    
    // Literals and identifiers
    IDENTIFIER,
    LITERAL,
    
    // Additional (add as needed)
    EMPTY
};

struct ASTNode {
    ASTNodeType type;
    std::string value;
    SourcePosition position;
    std::vector<std::shared_ptr<ASTNode>> children;

    ASTNode(ASTNodeType t) : type(t) {}
    ASTNode(ASTNodeType t, const std::string& v) : type(t), value(v) {}
    ASTNode(ASTNodeType t, const std::string& v, SourcePosition pos) 
        : type(t), value(v), position(pos) {}
};

} // namespace SCERSE
