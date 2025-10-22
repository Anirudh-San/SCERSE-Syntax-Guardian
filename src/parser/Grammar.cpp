#include "Grammar.hpp"
#include <iostream>

namespace SCERSE {

Grammar::Grammar() {
    // ========================================
    // STEP 1: Define the augmented start symbol and original start symbol
    // ========================================
    GrammarSymbol AugmentedStart("AugmentedStart");  // augmented start symbol
    GrammarSymbol Program("Program");

    // ========================================
    // STEP 2: Define ALL other non-terminals
    // ========================================
    GrammarSymbol StmtList("StmtList");
    GrammarSymbol Stmt("Stmt");
    GrammarSymbol VarDecl("VarDecl");
    GrammarSymbol FuncDecl("FuncDecl");
    GrammarSymbol Type("Type");
    GrammarSymbol Block("Block");
    GrammarSymbol ParamList("ParamList");
    GrammarSymbol Param("Param");
    GrammarSymbol Expr("Expr");
    GrammarSymbol Term("Term");
    GrammarSymbol Factor("Factor");
    GrammarSymbol ReturnStmt("ReturnStmt");

    // ========================================
    // STEP 3: Define ALL terminals
    // ========================================
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
    
    GrammarSymbol SEMICOLON("SEMICOLON", TokenType::SEMICOLON);
    GrammarSymbol COMMA("COMMA", TokenType::COMMA);
    GrammarSymbol DOT("DOT", TokenType::DOT);
    GrammarSymbol LPAREN("LPAREN", TokenType::LEFT_PAREN);
    GrammarSymbol RPAREN("RPAREN", TokenType::RIGHT_PAREN);
    GrammarSymbol LBRACE("LBRACE", TokenType::LEFT_BRACE);
    GrammarSymbol RBRACE("RBRACE", TokenType::RIGHT_BRACE);
    GrammarSymbol LBRACKET("LBRACKET", TokenType::LEFT_BRACKET);
    GrammarSymbol RBRACKET("RBRACKET", TokenType::RIGHT_BRACKET);

    // ========================================
    // STEP 4: Register nonTerminals and terminals
    // ========================================
    nonTerminals = {
        AugmentedStart, Program, StmtList, Stmt, VarDecl, FuncDecl, Type, Block,
        ParamList, Param, Expr, Term, Factor, ReturnStmt
    };

    terminals = {
        VAR, INT, FLOAT, STRING, BOOL, VOID, IF, ELSE, WHILE, FOR, RETURN,
        FUNCTION, CONST, TRUE_LIT, FALSE_LIT, IDENTIFIER, INTEGER, FLOAT_VAL,
        STRING_VAL, BOOLEAN, ASSIGN, PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
        EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
        AND, OR, NOT, SEMICOLON, COMMA, DOT, LPAREN, RPAREN, LBRACE,
        RBRACE, LBRACKET, RBRACKET
    };

    // ========================================
    // STEP 5: Add productions including augmented start production
    // ========================================

    addProduction(AugmentedStart, {Program}); // Augmented start production

    addProduction(Program, {StmtList});
    
    addProduction(StmtList, {Stmt, StmtList});
    addProduction(StmtList, {});  // ε - empty
    
    addProduction(Stmt, {VarDecl});
    addProduction(Stmt, {FuncDecl});
    addProduction(Stmt, {ReturnStmt});
    addProduction(Stmt, {Expr, SEMICOLON});
    
    addProduction(ReturnStmt, {RETURN, Expr, SEMICOLON});
    addProduction(ReturnStmt, {RETURN, SEMICOLON});
    
    addProduction(VarDecl, {Type, IDENTIFIER, SEMICOLON});
    addProduction(VarDecl, {Type, IDENTIFIER, ASSIGN, Expr, SEMICOLON});
    addProduction(VarDecl, {VAR, IDENTIFIER, ASSIGN, Expr, SEMICOLON});
    addProduction(VarDecl, {CONST, Type, IDENTIFIER, ASSIGN, Expr, SEMICOLON});
    
    addProduction(FuncDecl, {Type, IDENTIFIER, LPAREN, ParamList, RPAREN, Block});
    addProduction(FuncDecl, {Type, IDENTIFIER, LPAREN, RPAREN, Block});
    addProduction(FuncDecl, {VOID, IDENTIFIER, LPAREN, ParamList, RPAREN, Block});
    addProduction(FuncDecl, {VOID, IDENTIFIER, LPAREN, RPAREN, Block});
    
    addProduction(ParamList, {Param});
    addProduction(ParamList, {Param, COMMA, ParamList});
    
    addProduction(Param, {Type, IDENTIFIER});
    
    addProduction(Type, {INT});
    addProduction(Type, {FLOAT});
    addProduction(Type, {STRING});
    addProduction(Type, {BOOL});
    
    addProduction(Block, {LBRACE, StmtList, RBRACE});
    addProduction(Block, {LBRACE, RBRACE});
    
    addProduction(Expr, {Expr, PLUS, Term});
    addProduction(Expr, {Expr, MINUS, Term});
    addProduction(Expr, {Expr, EQUAL, Term});
    addProduction(Expr, {Expr, NOT_EQUAL, Term});
    addProduction(Expr, {Expr, LESS, Term});
    addProduction(Expr, {Expr, LESS_EQUAL, Term});
    addProduction(Expr, {Expr, GREATER, Term});
    addProduction(Expr, {Expr, GREATER_EQUAL, Term});
    addProduction(Expr, {Term});
    
    addProduction(Term, {Term, MULTIPLY, Factor});
    addProduction(Term, {Term, DIVIDE, Factor});
    addProduction(Term, {Term, MODULO, Factor});
    addProduction(Term, {Factor});
    
    addProduction(Factor, {INTEGER});
    addProduction(Factor, {FLOAT_VAL});
    addProduction(Factor, {STRING_VAL});
    addProduction(Factor, {TRUE_LIT});
    addProduction(Factor, {FALSE_LIT});
    addProduction(Factor, {IDENTIFIER});
    addProduction(Factor, {LPAREN, Expr, RPAREN});
    addProduction(Factor, {NOT, Factor});

    // ========================================
    // STEP 6: Compute FIRST and FOLLOW sets
    // ========================================
    computeFirstSets();
    computeFollowSets();
}

void Grammar::addProduction(const GrammarSymbol& lhs, const std::vector<GrammarSymbol>& rhs) {
    int id = static_cast<int>(productions.size());
    productions.emplace_back(lhs, rhs, id);
}

void Grammar::computeFirstSets() {
    // ======================================================
    // DEBUG 1: Check terminals
    // ======================================================
    //std::cout << "=== Starting computeFirstSets ===" << std::endl;
    //std::cout << "Terminals count: " << terminals.size() << std::endl;
    for (const auto& t : terminals) {
        //std::cout << "  Terminal: " << t.name << std::endl;
    }
    
    // Initialize FIRST sets for all terminals
    for (const auto& terminal : terminals) {
        firstSets[terminal].insert(terminal);
    }
    
    // Initialize FIRST sets for all non-terminals (empty at first)
    for (const auto& nonTerminal : nonTerminals) {
        if (firstSets.find(nonTerminal) == firstSets.end()) {
            firstSets[nonTerminal] = std::set<GrammarSymbol>();
        }
    }

    // Fixed epsilon symbol for consistent comparison
    GrammarSymbol epsilon("ε", TokenType::EOF_TOKEN);

    // ======================================================
    // DEBUG 2: Check before loop starts
    // ======================================================
    //std::cout << "Starting FIRST set computation..." << std::endl;
    //std::cout << "Productions count: " << productions.size() << std::endl;

    // Iterate until no changes (fixed-point iteration)
    bool changed = true;
    int iterations = 0;
    
    while (changed && iterations < 100) {
        changed = false;
        iterations++;
        
        // ======================================================
        // DEBUG 3: Iteration output
        // ======================================================
        //std::cout << "Iteration " << iterations << ": ";

        for (const auto& production : productions) {
            auto& lhsFirst = firstSets[production.lhs];
            size_t oldSize = lhsFirst.size();

            if (production.rhs.empty()) {
                // Empty production: A -> ε
                lhsFirst.insert(epsilon);
            } else {
                // Non-empty production: A -> B C D ...
                bool allCanBeEmpty = true;
                
                for (size_t i = 0; i < production.rhs.size(); ++i) {
                    const auto& symbol = production.rhs[i];
                    const auto& symbolFirst = firstSets[symbol];

                    // Add all non-epsilon symbols from this symbol's FIRST set
                    for (const auto& firstSymbol : symbolFirst) {
                        if (firstSymbol.name != "ε" && firstSymbol.name != "epsilon") {
                            lhsFirst.insert(firstSymbol);
                        }
                    }

                    // Check if this symbol can produce epsilon
                    bool hasEpsilon = false;
                    for (const auto& s : symbolFirst) {
                        if (s.name == "ε" || s.name == "epsilon") {
                            hasEpsilon = true;
                            break;
                        }
                    }

                    if (!hasEpsilon) {
                        allCanBeEmpty = false;
                        break;
                    }
                }

                // If all symbols in RHS can produce epsilon, add epsilon to LHS
                if (allCanBeEmpty) {
                    lhsFirst.insert(epsilon);
                }
            }

            // Check if FIRST set changed
            if (lhsFirst.size() != oldSize) {
                changed = true;
            }
        }
        
        // ======================================================
        // DEBUG 4: After each iteration
        // ======================================================
        //std::cout << (changed ? "CHANGED" : "NO CHANGE") << std::endl;
    }

    // ======================================================
    // DEBUG 5: Final FIRST sets
    // ======================================================
    //std::cout << "=== Final FIRST Sets ===" << std::endl;
    for (const auto& pair : firstSets) {
        //std::cout << "FIRST(" << pair.first.name << ") = { ";
        for (const auto& sym : pair.second) {
            std::cout << sym.name << " ";
        }
        //std::cout << "}" << std::endl;
    }
    //std::cout << "======================" << std::endl;
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
    // Direct lookup in firstSets map
    auto it = firstSets.find(symbol);
    if (it != firstSets.end()) {
        return it->second;
    }
    // If not found, return empty set
    return std::set<GrammarSymbol>();
}

std::set<GrammarSymbol> Grammar::getFirst(const std::vector<GrammarSymbol>& symbols) {
    std::set<GrammarSymbol> result;
    GrammarSymbol epsilon("ε", TokenType::EOF_TOKEN);

    // Empty sequence produces epsilon
    if (symbols.empty()) {
        result.insert(epsilon);
        return result;
    }

    // For each symbol in the sequence
    for (size_t i = 0; i < symbols.size(); ++i) {
        const auto& symbol = symbols[i];
        
        // DEBUG
        //std::cout << "    [getFirst] Processing symbol " << i << ": " << symbol.name << std::endl;
        
        const auto& symbolFirst = getFirst(symbol);  // Get FIRST(symbol)
        
        // DEBUG
        //std::cout << "      FIRST(" << symbol.name << ") size = " << symbolFirst.size() << std::endl;

        // Add all non-epsilon symbols from FIRST(symbol)
        bool hasEpsilon = false;
        for (const auto& firstSymbol : symbolFirst) {
            if (firstSymbol.name == "ε" || firstSymbol.name == "epsilon") {
                hasEpsilon = true;
            } else {
                result.insert(firstSymbol);
            }
        }

        // If this symbol doesn't produce epsilon, stop here
        if (!hasEpsilon) {
            // DEBUG
            //std::cout << "      " << symbol.name << " doesn't have epsilon, stopping" << std::endl;
            return result;
        }
    }

    // If we got here, ALL symbols produce epsilon
    result.insert(epsilon);
    return result;
}


std::set<GrammarSymbol> Grammar::getFollow(const GrammarSymbol& symbol) {
    return followSets[symbol];
}

bool Grammar::isNonTerminal(const GrammarSymbol& symbol) const {
    return nonTerminals.find(symbol) != nonTerminals.end();
}

bool Grammar::isTerminal(const GrammarSymbol& symbol) const {
    return terminals.find(symbol) != terminals.end();
}

} // namespace SCERSE
