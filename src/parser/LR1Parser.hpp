#pragma once

#include <vector>
#include <map>
#include <set>
#include <stack>
#include <memory>

#include "../lexer/Token.hpp"
#include "../common/AST.hpp"
#include "../common/Error.hpp"
#include "Grammar.hpp"

namespace SCERSE {

struct ParseResult {
    std::shared_ptr<ASTNode> ast;
    std::vector<CompilerError> errors;
    bool success = true;  // Added for compatibility with your code
};

class LR1Parser {
private:
    Grammar grammar;
    
    // Parsing table: state -> (symbol -> action)
    std::map<int, std::map<Symbol, Action>> actionTable;
    std::map<int, std::map<Symbol, int>> gotoTable;
    
    // LR(1) states
    std::vector<std::set<LR1Item>> states;
    
    void buildParsingTable();
    std::set<LR1Item> closure(const std::set<LR1Item>& items);
    std::set<LR1Item> gotoState(const std::set<LR1Item>& items, const Symbol& symbol);
    int findOrAddState(const std::set<LR1Item>& state);
    
    Symbol tokenToSymbol(const Token& token);
    std::shared_ptr<ASTNode> buildAST(const std::vector<std::shared_ptr<ASTNode>>& children, int productionId);
    
public:
    LR1Parser();
    ParseResult parse(const std::vector<Token>& tokens);
};

} // namespace SCERSE
