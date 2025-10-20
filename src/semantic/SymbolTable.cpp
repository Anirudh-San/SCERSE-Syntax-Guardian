#include "SymbolTable.hpp"
#include "../common/AST.hpp"
#include <algorithm>


namespace SCERSE {

SymbolTable::SymbolTable() : currentScopeLevel(0) {
    scopes.emplace_back(); // global scope
}

void SymbolTable::enterScope() {
    scopes.emplace_back();
    currentScopeLevel++;
}

void SymbolTable::exitScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
        currentScopeLevel--;
    }
}

bool SymbolTable::declareSymbol(const std::string& name, const Symbol& symbol) {
    auto &currentScope = scopes.back();
    if (currentScope.find(name) != currentScope.end())
        return false; // already declared in this scope
    currentScope[name] = symbol;
    return true;
}

Symbol* SymbolTable::lookupSymbol(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end())
            return &found->second;
    }
    return nullptr;
}

std::vector<Symbol> SymbolTable::getAllSymbols() const {
    std::vector<Symbol> all;
    for (const auto& scope : scopes) {
        for (const auto& pair : scope)
            all.push_back(pair.second);
    }
    return all;
}

void SymbolTable::clear() {
    scopes.clear();
    scopes.emplace_back(); // re-create global scope
    currentScopeLevel = 0;
}

// ===================================================================
// Main entry point: Build symbol table from AST root
// ===================================================================
void SymbolTable::buildFromAST(const std::shared_ptr<ASTNode>& root) {
    clear(); // Start fresh
    if (!root) return;
    
    processNode(root);
}

// ===================================================================
// Process any AST node recursively
// ===================================================================
void SymbolTable::processNode(const std::shared_ptr<ASTNode>& node) {
    if (!node) return;

    switch (node->type) {
        case ASTNodeType::PROGRAM:
            for (const auto& child : node->children) {
                processNode(child);
            }
            break;

        case ASTNodeType::VARIABLE_DECLARATION:
            processVariableDeclaration(node);
            break;

        case ASTNodeType::FUNCTION_DECLARATION:
            processFunctionDeclaration(node);
            break;

        case ASTNodeType::BLOCK_STATEMENT:
            processBlockStatement(node);
            break;

        case ASTNodeType::IF_STATEMENT:
        case ASTNodeType::WHILE_STATEMENT:
        case ASTNodeType::FOR_STATEMENT:
        case ASTNodeType::RETURN_STATEMENT:
        case ASTNodeType::EXPRESSION_STATEMENT:
        case ASTNodeType::ASSIGNMENT:
        case ASTNodeType::BINARY_OPERATION:
        case ASTNodeType::UNARY_OPERATION:
        case ASTNodeType::FUNCTION_CALL:
        case ASTNodeType::IDENTIFIER:
        case ASTNodeType::LITERAL:
            for (const auto& child : node->children) {
                processNode(child);
            }
            break;

        default:
            for (const auto& child : node->children) {
                processNode(child);
            }
            break;
    }
}

// ===================================================================
// Process variable declaration
// ===================================================================
void SymbolTable::processVariableDeclaration(const std::shared_ptr<ASTNode>& node) {
    if (!node || node->children.empty()) return;

    std::string varName;
    DataType varType = DataType::INTEGER;

    for (const auto& child : node->children) {
        if (child->type == ASTNodeType::IDENTIFIER) {
            varName = child->value;
        }
        else if (child->type == ASTNodeType::TYPE_SPECIFIER) {
            if (child->value == "int") varType = DataType::INTEGER;
            else if (child->value == "float") varType = DataType::FLOAT;
            else if (child->value == "bool") varType = DataType::BOOLEAN;
            else if (child->value == "string") varType = DataType::STRING;
        }
    }

    if (!varName.empty()) {
        Symbol symbol;
        symbol.name = varName;
        symbol.type = varType;
        symbol.symbolType = SymbolType::VARIABLE;
        symbol.scopeLevel = currentScopeLevel;

        declareSymbol(varName, symbol);
    }
}

// ===================================================================
// Process function declaration
// ===================================================================
void SymbolTable::processFunctionDeclaration(const std::shared_ptr<ASTNode>& node) {
    if (!node || node->children.empty()) return;

    std::string funcName;
    DataType returnType = DataType::VOID;

    for (const auto& child : node->children) {
        if (child->type == ASTNodeType::IDENTIFIER) {
            funcName = child->value;
        }
        else if (child->type == ASTNodeType::TYPE_SPECIFIER) {
            if (child->value == "int") returnType = DataType::INTEGER;
            else if (child->value == "float") returnType = DataType::FLOAT;
            else if (child->value == "bool") returnType = DataType::BOOLEAN;
            else if (child->value == "string") returnType = DataType::STRING;
        }
    }

    if (!funcName.empty()) {
        Symbol symbol;
        symbol.name = funcName;
        symbol.type = returnType;
        symbol.symbolType = SymbolType::FUNCTION;
        symbol.scopeLevel = currentScopeLevel;

        declareSymbol(funcName, symbol);
    }

    enterScope();

    for (const auto& child : node->children) {
        if (child->type == ASTNodeType::PARAMETER_LIST) {
            for (const auto& param : child->children) {
                processVariableDeclaration(param);
            }
        }
        else if (child->type == ASTNodeType::BLOCK_STATEMENT) {
            processBlockStatement(child);
        }
    }

    exitScope();
}

// ===================================================================
// Process block statement
// ===================================================================
void SymbolTable::processBlockStatement(const std::shared_ptr<ASTNode>& node) {
    if (!node) return;

    enterScope();

    for (const auto& child : node->children) {
        processNode(child);
    }

    exitScope();
}

} // namespace SCERSE
