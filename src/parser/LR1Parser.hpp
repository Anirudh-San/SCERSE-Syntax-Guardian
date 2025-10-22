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
    bool success = true;
};


class LR1Parser {
public:
    LR1Parser();
    ParseResult parse(const std::vector<Token>& tokens);


    struct ParserLR1Item {
    int productionId;
    int dotPosition;
    GrammarSymbol lookahead;
    
    // ADD THIS CONSTRUCTOR
    ParserLR1Item() = default;
    
    ParserLR1Item(int prodId, int dotPos, const GrammarSymbol& la)
        : productionId(prodId), dotPosition(dotPos), lookahead(la) {}
    
    // CRITICAL: Proper comparison for std::set - COMPARE BY NAME ONLY
    bool operator<(const ParserLR1Item& other) const {
        if (productionId != other.productionId) 
            return productionId < other.productionId;
        if (dotPosition != other.dotPosition) 
            return dotPosition < other.dotPosition;
        // Compare lookahead by name ONLY (not by tokenType)
        return lookahead.name < other.lookahead.name;
    }
    
    bool operator==(const ParserLR1Item& other) const {
        return productionId == other.productionId && 
               dotPosition == other.dotPosition && 
               lookahead.name == other.lookahead.name;  // Compare by name only
    }
};




private:


    Grammar grammar;


    std::map<int, std::map<GrammarSymbol, Action>> actionTable;
    std::map<int, std::map<GrammarSymbol, int>> gotoTable;


    std::vector<std::set<ParserLR1Item>> states;


    void buildParsingTable();


    std::set<ParserLR1Item> closure(const std::set<ParserLR1Item>& items);
    std::set<ParserLR1Item> gotoState(const std::set<ParserLR1Item>& items, const GrammarSymbol& symbol);


    int findOrAddState(const std::set<ParserLR1Item>& state);


    GrammarSymbol tokenToGrammarSymbol(const Token& token) const;
    std::shared_ptr<ASTNode> buildAST(const std::vector<std::shared_ptr<ASTNode>>& children, int productionId);
};


} // namespace SCERSE
