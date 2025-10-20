#include "Grammar.hpp"

namespace SCERSE {

Grammar::Grammar() {
    // Define the start symbol
    startSymbol = Symbol("Program");
    nonTerminals.insert(startSymbol);
    
    // Define terminals using YOUR actual TokenType enum values
    Symbol VAR("VAR", TokenType::VAR);
    Symbol IDENTIFIER("IDENTIFIER", TokenType::IDENTIFIER);
    Symbol INT("INT", TokenType::INT);
    Symbol FLOAT("FLOAT", TokenType::FLOAT_KW);
    Symbol STRING("STRING", TokenType::STRING_KW);
    Symbol BOOL("BOOL", TokenType::BOOL);
    Symbol ASSIGN("ASSIGN", TokenType::ASSIGN);
    Symbol SEMICOLON("SEMICOLON", TokenType::SEMICOLON);
    Symbol INTEGER("INTEGER", TokenType::INTEGER);
    Symbol FLOAT_VAL("FLOAT_VAL", TokenType::FLOAT);
    Symbol PLUS("PLUS", TokenType::PLUS);
    Symbol MINUS("MINUS", TokenType::MINUS);
    Symbol MULTIPLY("MULTIPLY", TokenType::MULTIPLY);
    Symbol DIVIDE("DIVIDE", TokenType::DIVIDE);
    Symbol LPAREN("LPAREN", TokenType::LEFT_PAREN);
    Symbol RPAREN("RPAREN", TokenType::RIGHT_PAREN);
    Symbol EOF_SYM("$", TokenType::EOF_TOKEN);
    
    terminals.insert(VAR);
    terminals.insert(IDENTIFIER);
    terminals.insert(INT);
    terminals.insert(FLOAT);
    terminals.insert(STRING);
    terminals.insert(BOOL);
    terminals.insert(ASSIGN);
    terminals.insert(SEMICOLON);
    terminals.insert(INTEGER);
    terminals.insert(FLOAT_VAL);
    terminals.insert(PLUS);
    terminals.insert(MINUS);
    terminals.insert(MULTIPLY);
    terminals.insert(DIVIDE);
    terminals.insert(LPAREN);
    terminals.insert(RPAREN);
    terminals.insert(EOF_SYM);
    
    // Define non-terminals
    Symbol Program("Program");
    Symbol StmtList("StmtList");
    Symbol Stmt("Stmt");
    Symbol VarDecl("VarDecl");
    Symbol Type("Type");
    Symbol Expr("Expr");
    Symbol Term("Term");
    Symbol Factor("Factor");
    
    nonTerminals.insert(Program);
    nonTerminals.insert(StmtList);
    nonTerminals.insert(Stmt);
    nonTerminals.insert(VarDecl);
    nonTerminals.insert(Type);
    nonTerminals.insert(Expr);
    nonTerminals.insert(Term);
    nonTerminals.insert(Factor);
    
    // Define grammar productions
    // 0: Program -> StmtList
    addProduction(Program, {StmtList});
    
    // 1: StmtList -> Stmt StmtList
    addProduction(StmtList, {Stmt, StmtList});
    
    // 2: StmtList -> ε (empty)
    addProduction(StmtList, {});
    
    // 3: Stmt -> VarDecl
    addProduction(Stmt, {VarDecl});
    
    // 4: VarDecl -> VAR IDENTIFIER Type SEMICOLON
    addProduction(VarDecl, {VAR, IDENTIFIER, Type, SEMICOLON});
    
    // 5: VarDecl -> VAR IDENTIFIER Type ASSIGN Expr SEMICOLON
    addProduction(VarDecl, {VAR, IDENTIFIER, Type, ASSIGN, Expr, SEMICOLON});
    
    // 6: Type -> INT
    addProduction(Type, {INT});
    
    // 7: Type -> FLOAT
    addProduction(Type, {FLOAT});
    
    // 8: Type -> STRING
    addProduction(Type, {STRING});
    
    // 9: Type -> BOOL
    addProduction(Type, {BOOL});
    
    // 10: Expr -> Expr PLUS Term
    addProduction(Expr, {Expr, PLUS, Term});
    
    // 11: Expr -> Expr MINUS Term
    addProduction(Expr, {Expr, MINUS, Term});
    
    // 12: Expr -> Term
    addProduction(Expr, {Term});
    
    // 13: Term -> Term MULTIPLY Factor
    addProduction(Term, {Term, MULTIPLY, Factor});
    
    // 14: Term -> Term DIVIDE Factor
    addProduction(Term, {Term, DIVIDE, Factor});
    
    // 15: Term -> Factor
    addProduction(Term, {Factor});
    
    // 16: Factor -> INTEGER
    addProduction(Factor, {INTEGER});
    
    // 17: Factor -> FLOAT_VAL
    addProduction(Factor, {FLOAT_VAL});
    
    // 18: Factor -> IDENTIFIER
    addProduction(Factor, {IDENTIFIER});
    
    // 19: Factor -> LPAREN Expr RPAREN
    addProduction(Factor, {LPAREN, Expr, RPAREN});
    
    // Compute FIRST and FOLLOW sets
    computeFirstSets();
    computeFollowSets();
}

void Grammar::addProduction(const Symbol& lhs, const std::vector<Symbol>& rhs) {
    int id = productions.size();
    productions.push_back(Production(lhs, rhs, id));
}

void Grammar::computeFirstSets() {
    for (const auto& terminal : terminals) {
        firstSets[terminal].insert(terminal);
    }
    
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& production : productions) {
            auto& lhsFirst = firstSets[production.lhs];
            size_t oldSize = lhsFirst.size();
            
            if (production.rhs.empty()) {
                lhsFirst.insert(Symbol("ε", TokenType::EOF_TOKEN));
            } else {
                for (const auto& symbol : production.rhs) {
                    auto& symbolFirst = firstSets[symbol];
                    for (const auto& firstSymbol : symbolFirst) {
                        if (firstSymbol.name != "ε") {
                            lhsFirst.insert(firstSymbol);
                        }
                    }
                    if (symbolFirst.find(Symbol("ε", TokenType::EOF_TOKEN)) == symbolFirst.end()) {
                        break;
                    }
                }
            }
            
            if (lhsFirst.size() != oldSize) changed = true;
        }
    }
}

void Grammar::computeFollowSets() {
    followSets[startSymbol].insert(Symbol("$", TokenType::EOF_TOKEN));
    
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& production : productions) {
            for (size_t i = 0; i < production.rhs.size(); ++i) {
                const Symbol& B = production.rhs[i];
                
                if (isNonTerminal(B)) {
                    auto& followB = followSets[B];
                    size_t oldSize = followB.size();
                    
                    std::vector<Symbol> beta(production.rhs.begin() + i + 1, production.rhs.end());
                    auto firstBeta = getFirst(beta);
                    
                    for (const auto& symbol : firstBeta) {
                        if (symbol.name != "ε") {
                            followB.insert(symbol);
                        }
                    }
                    
                    if (beta.empty() || firstBeta.find(Symbol("ε", TokenType::EOF_TOKEN)) != firstBeta.end()) {
                        auto& followA = followSets[production.lhs];
                        followB.insert(followA.begin(), followA.end());
                    }
                    
                    if (followB.size() != oldSize) changed = true;
                }
            }
        }
    }
}

std::set<Symbol> Grammar::getFirst(const Symbol& symbol) {
    return firstSets[symbol];
}

std::set<Symbol> Grammar::getFirst(const std::vector<Symbol>& symbols) {
    std::set<Symbol> result;
    
    if (symbols.empty()) {
        result.insert(Symbol("ε", TokenType::EOF_TOKEN));
        return result;
    }
    
    for (const auto& symbol : symbols) {
        auto& symbolFirst = firstSets[symbol];
        for (const auto& firstSymbol : symbolFirst) {
            if (firstSymbol.name != "ε") {
                result.insert(firstSymbol);
            }
        }
        if (symbolFirst.find(Symbol("ε", TokenType::EOF_TOKEN)) == symbolFirst.end()) {
            return result;
        }
    }
    
    result.insert(Symbol("ε", TokenType::EOF_TOKEN));
    return result;
}

std::set<Symbol> Grammar::getFollow(const Symbol& symbol) {
    return followSets[symbol];
}

bool Grammar::isTerminal(const Symbol& symbol) const {
    return terminals.find(symbol) != terminals.end();
}

bool Grammar::isNonTerminal(const Symbol& symbol) const {
    return nonTerminals.find(symbol) != nonTerminals.end();
}

} // namespace SCERSE
