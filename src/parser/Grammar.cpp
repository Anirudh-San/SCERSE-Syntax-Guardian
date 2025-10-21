#include "Grammar.hpp"

namespace SCERSE {

Grammar::Grammar() {
    // Define the start symbol
    startSymbol = GrammarSymbol("Program");
    nonTerminals.insert(startSymbol);

    // Define ALL terminals (must match tokenToGrammarSymbol)
    GrammarSymbol VAR("VAR", TokenType::VAR);
    GrammarSymbol INT("INT", TokenType::INT);
    GrammarSymbol FLOAT("FLOAT", TokenType::FLOAT_KW);
    GrammarSymbol STRING("STRING", TokenType::STRING_KW);
    GrammarSymbol BOOL("BOOL", TokenType::BOOL);
    GrammarSymbol VOID("VOID", TokenType::VOID);
    GrammarSymbol IF("IF", TokenType::IF);
    GrammarSymbol ELSE("ELSE", TokenType::ELSE);
    GrammarSymbol WHILE("WHILE", TokenType::WHILE);
    GrammarSymbol FOR("FOR", TokenType::FOR);
    GrammarSymbol RETURN("RETURN", TokenType::RETURN);
    GrammarSymbol FUNCTION("FUNCTION", TokenType::FUNCTION);
    GrammarSymbol CONST("CONST", TokenType::CONST);
    GrammarSymbol TRUE_LIT("TRUE", TokenType::TRUE);
    GrammarSymbol FALSE_LIT("FALSE", TokenType::FALSE);
    
    GrammarSymbol IDENTIFIER("IDENTIFIER", TokenType::IDENTIFIER);
    GrammarSymbol INTEGER("INTEGER", TokenType::INTEGER);
    GrammarSymbol FLOAT_VAL("FLOAT_VAL", TokenType::FLOAT);
    GrammarSymbol STRING_VAL("STRING_VAL", TokenType::STRING);
    GrammarSymbol BOOLEAN("BOOLEAN", TokenType::BOOLEAN);
    
    GrammarSymbol ASSIGN("ASSIGN", TokenType::ASSIGN);
    GrammarSymbol SEMICOLON("SEMICOLON", TokenType::SEMICOLON);
    GrammarSymbol COMMA("COMMA", TokenType::COMMA);
    GrammarSymbol DOT("DOT", TokenType::DOT);
    
    GrammarSymbol PLUS("PLUS", TokenType::PLUS);
    GrammarSymbol MINUS("MINUS", TokenType::MINUS);
    GrammarSymbol MULTIPLY("MULTIPLY", TokenType::MULTIPLY);
    GrammarSymbol DIVIDE("DIVIDE", TokenType::DIVIDE);
    GrammarSymbol MODULO("MODULO", TokenType::MODULO);
    
    GrammarSymbol EQUAL("EQUAL", TokenType::EQUAL);
    GrammarSymbol NOT_EQUAL("NOT_EQUAL", TokenType::NOT_EQUAL);
    GrammarSymbol LESS("LESS", TokenType::LESS);
    GrammarSymbol LESS_EQUAL("LESS_EQUAL", TokenType::LESS_EQUAL);
    GrammarSymbol GREATER("GREATER", TokenType::GREATER);
    GrammarSymbol GREATER_EQUAL("GREATER_EQUAL", TokenType::GREATER_EQUAL);
    
    GrammarSymbol AND("AND", TokenType::LOGICAL_AND);
    GrammarSymbol OR("OR", TokenType::LOGICAL_OR);
    GrammarSymbol NOT("NOT", TokenType::LOGICAL_NOT);
    
    GrammarSymbol LPAREN("LPAREN", TokenType::LEFT_PAREN);
    GrammarSymbol RPAREN("RPAREN", TokenType::RIGHT_PAREN);
    GrammarSymbol LBRACE("LBRACE", TokenType::LEFT_BRACE);
    GrammarSymbol RBRACE("RBRACE", TokenType::RIGHT_BRACE);
    GrammarSymbol LBRACKET("LBRACKET", TokenType::LEFT_BRACKET);
    GrammarSymbol RBRACKET("RBRACKET", TokenType::RIGHT_BRACKET);
    
    GrammarSymbol EOF_SYM("$", TokenType::EOF_TOKEN);

    terminals = {
        VAR, INT, FLOAT, STRING, BOOL, VOID,
        IF, ELSE, WHILE, FOR, RETURN, FUNCTION, CONST, TRUE_LIT, FALSE_LIT,
        IDENTIFIER, INTEGER, FLOAT_VAL, STRING_VAL, BOOLEAN,
        ASSIGN, SEMICOLON, COMMA, DOT,
        PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
        EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
        AND, OR, NOT,
        LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
        EOF_SYM
    };

    // Define non-terminals
    GrammarSymbol Program("Program");
    GrammarSymbol StmtList("StmtList");
    GrammarSymbol Stmt("Stmt");
    GrammarSymbol VarDecl("VarDecl");
    GrammarSymbol FuncDecl("FuncDecl");
    GrammarSymbol Type("Type");
    GrammarSymbol Block("Block");
    GrammarSymbol Expr("Expr");
    GrammarSymbol Term("Term");
    GrammarSymbol Factor("Factor");

    nonTerminals = {Program, StmtList, Stmt, VarDecl, FuncDecl, Type, Block, Expr, Term, Factor};

    // Grammar productions for basic C-style syntax
    addProduction(Program, {StmtList});
    addProduction(StmtList, {Stmt, StmtList});
    addProduction(StmtList, {}); // ε

    addProduction(Stmt, {VarDecl});
    addProduction(Stmt, {FuncDecl});
    addProduction(Stmt, {RETURN, Expr, SEMICOLON});
    addProduction(Stmt, {RETURN, SEMICOLON});

    // Variable declarations: int x; or int x = 5;
    addProduction(VarDecl, {Type, IDENTIFIER, SEMICOLON});
    addProduction(VarDecl, {Type, IDENTIFIER, ASSIGN, Expr, SEMICOLON});
    
    // Also support: var x int;
    addProduction(VarDecl, {VAR, IDENTIFIER, Type, SEMICOLON});
    addProduction(VarDecl, {VAR, IDENTIFIER, Type, ASSIGN, Expr, SEMICOLON});

    // Function declarations: int main() { }
    addProduction(FuncDecl, {Type, IDENTIFIER, LPAREN, RPAREN, Block});

    addProduction(Type, {INT});
    addProduction(Type, {FLOAT});
    addProduction(Type, {STRING});
    addProduction(Type, {BOOL});
    addProduction(Type, {VOID});

    addProduction(Block, {LBRACE, StmtList, RBRACE});

    addProduction(Expr, {Expr, PLUS, Term});
    addProduction(Expr, {Expr, MINUS, Term});
    addProduction(Expr, {Term});

    addProduction(Term, {Term, MULTIPLY, Factor});
    addProduction(Term, {Term, DIVIDE, Factor});
    addProduction(Term, {Factor});

    addProduction(Factor, {INTEGER});
    addProduction(Factor, {FLOAT_VAL});
    addProduction(Factor, {TRUE_LIT});
    addProduction(Factor, {FALSE_LIT});
    addProduction(Factor, {IDENTIFIER});
    addProduction(Factor, {LPAREN, Expr, RPAREN});

    // Compute FIRST and FOLLOW sets
    computeFirstSets();
    computeFollowSets();
}
void Grammar::addProduction(const GrammarSymbol& lhs, const std::vector<GrammarSymbol>& rhs) {
    int id = static_cast<int>(productions.size());
    productions.emplace_back(lhs, rhs, id);
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
                lhsFirst.insert(GrammarSymbol("ε", TokenType::EOF_TOKEN));
            } else {
                for (const auto& symbol : production.rhs) {
                    auto& symbolFirst = firstSets[symbol];
                    for (const auto& firstSymbol : symbolFirst) {
                        if (firstSymbol.name != "ε") {
                            lhsFirst.insert(firstSymbol);
                        }
                    }
                    if (symbolFirst.find(GrammarSymbol("ε", TokenType::EOF_TOKEN)) == symbolFirst.end()) {
                        break;
                    }
                }
            }

            if (lhsFirst.size() != oldSize) changed = true;
        }
    }
}

void Grammar::computeFollowSets() {
    followSets[startSymbol].insert(GrammarSymbol("$", TokenType::EOF_TOKEN));

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& production : productions) {
            for (size_t i = 0; i < production.rhs.size(); ++i) {
                const auto& B = production.rhs[i];

                if (isNonTerminal(B)) {
                    auto& followB = followSets[B];
                    size_t oldSize = followB.size();

                    std::vector<GrammarSymbol> beta(production.rhs.begin() + i + 1, production.rhs.end());
                    auto firstBeta = getFirst(beta);

                    for (const auto& symbol : firstBeta) {
                        if (symbol.name != "ε") {
                            followB.insert(symbol);
                        }
                    }

                    if (beta.empty() || firstBeta.find(GrammarSymbol("ε", TokenType::EOF_TOKEN)) != firstBeta.end()) {
                        const auto& followA = followSets[production.lhs];
                        followB.insert(followA.begin(), followA.end());
                    }

                    if (followB.size() != oldSize) changed = true;
                }
            }
        }
    }
}

std::set<GrammarSymbol> Grammar::getFirst(const GrammarSymbol& symbol) {
    return firstSets[symbol];
}

std::set<GrammarSymbol> Grammar::getFirst(const std::vector<GrammarSymbol>& symbols) {
    std::set<GrammarSymbol> result;

    if (symbols.empty()) {
        result.insert(GrammarSymbol("ε", TokenType::EOF_TOKEN));
        return result;
    }

    for (const auto& symbol : symbols) {
        const auto& symbolFirst = firstSets[symbol];
        for (const auto& firstSymbol : symbolFirst) {
            if (firstSymbol.name != "ε") {
                result.insert(firstSymbol);
            }
        }
        if (symbolFirst.find(GrammarSymbol("ε", TokenType::EOF_TOKEN)) == symbolFirst.end()) {
            return result;
        }
    }

    result.insert(GrammarSymbol("ε", TokenType::EOF_TOKEN));
    return result;
}

std::set<GrammarSymbol> Grammar::getFollow(const GrammarSymbol& symbol) {
    return followSets[symbol];
}

bool Grammar::isTerminal(const GrammarSymbol& symbol) const {
    return terminals.find(symbol) != terminals.end();
}

bool Grammar::isNonTerminal(const GrammarSymbol& symbol) const {
    return nonTerminals.find(symbol) != nonTerminals.end();
}

} // namespace SCERSE
