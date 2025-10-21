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

private:
    // Parser-local LR(1) item type to avoid name collisions with other headers
    struct ParserLR1Item {
        int productionId;
        size_t dotPosition;
        GrammarSymbol lookahead;

        ParserLR1Item(int pid = 0, size_t dot = 0, const GrammarSymbol& la = GrammarSymbol())
            : productionId(pid), dotPosition(dot), lookahead(la) {}

        bool operator<(ParserLR1Item const& o) const {
            if (productionId != o.productionId) return productionId < o.productionId;
            if (dotPosition != o.dotPosition) return dotPosition < o.dotPosition;
            return lookahead < o.lookahead; // relies on GrammarSymbol::operator<
        }

        bool operator==(ParserLR1Item const& o) const {
            return productionId == o.productionId &&
                   dotPosition == o.dotPosition &&
                   lookahead == o.lookahead;
        }
    };

    // Grammar instance (owned)
    Grammar grammar;

    // Parsing tables
    std::map<int, std::map<GrammarSymbol, Action>> actionTable;
    std::map<int, std::map<GrammarSymbol, int>> gotoTable;

    // Canonical collection of LR(1) item sets (states)
    std::vector<std::set<ParserLR1Item>> states;

    // Core algorithms
    void buildParsingTable();
    std::set<ParserLR1Item> closure(const std::set<ParserLR1Item>& items);
    std::set<ParserLR1Item> gotoState(const std::set<ParserLR1Item>& items, const GrammarSymbol& symbol);
    int findOrAddState(const std::set<ParserLR1Item>& state);

    // Helpers
    GrammarSymbol tokenToGrammarSymbol(const Token& token) const;
    std::shared_ptr<ASTNode> buildAST(const std::vector<std::shared_ptr<ASTNode>>& children, int productionId);
};

} // namespace SCERSE
