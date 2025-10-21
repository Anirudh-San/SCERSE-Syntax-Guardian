#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include "../lexer/Token.hpp"
#include "../common/Types.hpp"

namespace SCERSE {

// Grammar symbols (renamed to avoid conflict with SymbolTable.hpp)
enum class GrammarSymbolType {
    TERMINAL,
    NON_TERMINAL
};

struct GrammarSymbol {
    GrammarSymbolType type;
    std::string name;
    TokenType tokenType; // For terminals

    GrammarSymbol()
        : type(GrammarSymbolType::TERMINAL), tokenType(TokenType::EOF_TOKEN) {}

    GrammarSymbol(const std::string& n, TokenType tt)
        : type(GrammarSymbolType::TERMINAL), name(n), tokenType(tt) {}

    GrammarSymbol(const std::string& n)
        : type(GrammarSymbolType::NON_TERMINAL), name(n), tokenType(TokenType::EOF_TOKEN) {}

    bool operator<(const GrammarSymbol& other) const {
        return name < other.name;
    }

    bool operator==(const GrammarSymbol& other) const {
        return name == other.name && type == other.type;
    }
};

// Production rule: LHS -> RHS
struct Production {
    GrammarSymbol lhs;
    std::vector<GrammarSymbol> rhs;
    int id;

    Production(const GrammarSymbol& l, const std::vector<GrammarSymbol>& r, int i)
        : lhs(l), rhs(r), id(i) {}
};

// LR(1) item: [A -> α·β, lookahead]
struct LR1Item {
    int productionId;
    int dotPosition;
    GrammarSymbol lookahead;

    LR1Item(int pid, int dot, const GrammarSymbol& la)
        : productionId(pid), dotPosition(dot), lookahead(la) {}

    bool operator<(const LR1Item& other) const {
        if (productionId != other.productionId)
            return productionId < other.productionId;
        if (dotPosition != other.dotPosition)
            return dotPosition < other.dotPosition;
        return lookahead < other.lookahead;
    }

    bool operator==(const LR1Item& other) const {
        return productionId == other.productionId &&
               dotPosition == other.dotPosition &&
               lookahead == other.lookahead;
    }
};

struct Action {
    ActionType type;
    int value; // State number for SHIFT, production id for REDUCE

    Action() : type(ActionType::ERROR), value(-1) {}
    Action(ActionType t, int v) : type(t), value(v) {}
};

class Grammar {
private:
    std::vector<Production> productions;
    GrammarSymbol startSymbol;
    std::set<GrammarSymbol> terminals;
    std::set<GrammarSymbol> nonTerminals;

    // FIRST and FOLLOW sets
    std::map<GrammarSymbol, std::set<GrammarSymbol>> firstSets;
    std::map<GrammarSymbol, std::set<GrammarSymbol>> followSets;

    void computeFirstSets();
    void computeFollowSets();

public:
    Grammar();

    void addProduction(const GrammarSymbol& lhs, const std::vector<GrammarSymbol>& rhs);
    const std::vector<Production>& getProductions() const { return productions; }
    const Production& getProduction(int id) const { return productions[id]; }
    const GrammarSymbol& getStartSymbol() const { return startSymbol; }

    std::set<GrammarSymbol> getFirst(const GrammarSymbol& symbol);
    std::set<GrammarSymbol> getFirst(const std::vector<GrammarSymbol>& symbols);
    std::set<GrammarSymbol> getFollow(const GrammarSymbol& symbol);

    bool isTerminal(const GrammarSymbol& symbol) const;
    bool isNonTerminal(const GrammarSymbol& symbol) const;
};

} // namespace SCERSE
