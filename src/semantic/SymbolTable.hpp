#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "../common/Types.hpp"
#include "../common/AST.hpp"

namespace SCERSE {

struct SemanticSymbol {
    std::string name;
    DataType type;
    int scopeLevel;
};


class SymbolTable {
private:
    std::vector<std::unordered_map<std::string, SemanticSymbol>> scopes;
    int currentScopeLevel;

    void processNode(const std::shared_ptr<ASTNode>& node);
    void processVariableDeclaration(const std::shared_ptr<ASTNode>& node);
    void processFunctionDeclaration(const std::shared_ptr<ASTNode>& node);
    void processBlockStatement(const std::shared_ptr<ASTNode>& node);

public:
    SymbolTable();

    void enterScope();
    void exitScope();
    bool declareSymbol(const std::string& name, const SemanticSymbol& symbol);
    SemanticSymbol* lookupSymbol(const std::string& name);
    std::vector<SemanticSymbol> getAllSymbols() const;
    
    void buildFromAST(const std::shared_ptr<ASTNode>& root);
    void clear();
};

} // namespace SCERSE
