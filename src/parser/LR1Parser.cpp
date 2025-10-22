#include "LR1Parser.hpp"
#include <algorithm>
#include <iostream>

namespace SCERSE {

// In LR1Parser.cpp constructor
LR1Parser::LR1Parser() {
    std::cout << "=== LR1Parser Initialization ===" << std::endl;
    
    try {
        buildParsingTable();
        
        std::cout << "✓ States: " << states.size() << std::endl;
        std::cout << "✓ ACTION entries: " << actionTable.size() << std::endl;
        std::cout << "✓ GOTO entries: " << gotoTable.size() << std::endl;
        
        if (states.empty()) {
            std::cerr << "WARNING: No parser states generated!" << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "ERROR building parser table: " << ex.what() << std::endl;
    }
    
    std::cout << "=================================\n" << std::endl;
}

ParseResult LR1Parser::parse(const std::vector<Token>& tokensIn) {
    ParseResult result;
    result.success = true;
    
    for (const auto& token : tokensIn) {
        if (token.type == TokenType::ERROR_TOKEN) {
            result.success = false;
            result.errors.push_back(
                CompilerError(ErrorSeverity::ERROR,
                              "Unexpected or unknown token: " + token.lexeme,
                              Position(token.position.line, token.position.column))
            );
        }
    }
    
    if (!tokensIn.empty() && tokensIn.back().type != TokenType::EOF_TOKEN) {
        result.success = false;
        result.errors.push_back(
            CompilerError(ErrorSeverity::ERROR,
                          "Missing end-of-file token",
                          Position(tokensIn.back().position.line, tokensIn.back().position.column))
        );
    }
    
    if (states.empty()) {
        std::cerr << "Warning: Parser states table is empty - skipping syntax analysis\n";
        return result;
    }
    
    std::stack<int> stateStack;
    std::stack<std::shared_ptr<ASTNode>> nodeStack;
    stateStack.push(0);
    
    std::vector<Token> tokens = tokensIn;
    if (tokens.empty() || tokens.back().type != TokenType::EOF_TOKEN) {
        tokens.push_back(Token{TokenType::EOF_TOKEN, "$", Position()});
    }
    
    size_t idx = 0;
    int errorCount = 0;
    const int MAX_ERRORS = 50;
    
    while (idx < tokens.size() && errorCount < MAX_ERRORS) {
        if (stateStack.empty()) {
            result.errors.push_back(
                CompilerError(ErrorSeverity::ERROR,
                              "Parser state stack empty - cannot continue",
                              Position())
            );
            break;
        }
        
        int curState = stateStack.top();
        const Token& curToken = tokens[idx];
        GrammarSymbol curSym = tokenToGrammarSymbol(curToken);
        
        auto stIt = actionTable.find(curState);
        if (stIt == actionTable.end()) {
            result.success = false;
            result.errors.push_back(
                CompilerError(ErrorSeverity::ERROR,
                              "Syntax error near: " + curToken.lexeme,
                              Position(curToken.position.line, curToken.position.column))
            );
            
            ++idx;
            ++errorCount;
            continue;
        }
        
        auto actIt = stIt->second.find(curSym);
        if (actIt == stIt->second.end()) {
            result.success = false;
            result.errors.push_back(
                CompilerError(ErrorSeverity::ERROR,
                              "Unexpected token: " + curToken.lexeme,
                              Position(curToken.position.line, curToken.position.column))
            );
            
            ++idx;
            ++errorCount;
            continue;
        }
        
        Action action = actIt->second;
        
        switch (action.type) {
            case ActionType::SHIFT: {
                stateStack.push(action.value);
                auto node = std::make_shared<ASTNode>(ASTNodeType::LITERAL, curToken.lexeme);
                node->position = SourcePosition(curToken.position.line, curToken.position.column);
                nodeStack.push(node);
                ++idx;
                break;
            }
            
            case ActionType::REDUCE: {
                const auto& prod = grammar.getProduction(action.value);
                std::vector<std::shared_ptr<ASTNode>> children;
                
                for (size_t i = 0; i < prod.rhs.size(); ++i) {
                    if (!nodeStack.empty()) {
                        children.insert(children.begin(), nodeStack.top());
                        nodeStack.pop();
                    }
                    if (!stateStack.empty()) stateStack.pop();
                }
                
                auto node = buildAST(children, action.value);
                nodeStack.push(node);
                
                if (!stateStack.empty()) {
                    int topState = stateStack.top();
                    auto gotoIt = gotoTable.find(topState);
                    if (gotoIt != gotoTable.end()) {
                        auto symbolIt = gotoIt->second.find(prod.lhs);
                        if (symbolIt != gotoIt->second.end()) {
                            stateStack.push(symbolIt->second);
                        } else {
                            result.success = false;
                            result.errors.push_back(
                                CompilerError(ErrorSeverity::ERROR,
                                              "Parser table missing GOTO entry during reduce",
                                              Position())
                            );
                            ++idx;
                            ++errorCount;
                        }
                    } else {
                        result.success = false;
                        result.errors.push_back(
                            CompilerError(ErrorSeverity::ERROR,
                                          "Parser table missing GOTO map during reduce",
                                          Position())
                        );
                        ++idx;
                        ++errorCount;
                    }
                }
                break;
            }
            
            case ActionType::ACCEPT:
                if (!nodeStack.empty()) result.ast = nodeStack.top();
                result.success = (errorCount == 0);
                return result;
                
            case ActionType::ERROR:
            default:
                result.success = false;
                result.errors.push_back(
                    CompilerError(ErrorSeverity::ERROR,
                                  "Parse error at token: " + curToken.lexeme,
                                  Position(curToken.position.line, curToken.position.column))
                );
                ++idx;
                ++errorCount;
                break;
        }
    }
    
    if (errorCount >= MAX_ERRORS) {
        result.errors.push_back(
            CompilerError(ErrorSeverity::ERROR,
                          "Too many errors - stopping parse",
                          Position())
        );
    }
    
    return result;
}


void LR1Parser::buildParsingTable() {
    states.clear();
    actionTable.clear();
    gotoTable.clear();

    // Validate grammar has productions
    if (grammar.getProductionCount() == 0) {
        std::cerr << "ERROR: Grammar has no productions!" << std::endl;
        return;
    }

    try {
        // Create augmented start production: S' -> Program $
        ParserLR1Item startItem(0, 0, GrammarSymbol("$", TokenType::EOF_TOKEN));
        std::set<ParserLR1Item> startSet = closure({startItem});
        states.push_back(startSet);

        for (size_t si = 0; si < states.size(); ++si) {
            const auto& state = states[si];

            std::set<GrammarSymbol> symbolsAfterDot;
            for (const auto& it : state) {
                if (it.productionId >= grammar.getProductionCount()) {
                    std::cerr << "ERROR: Invalid production ID in state: " << it.productionId << std::endl;
                    continue;
                }
                const auto& prod = grammar.getProduction(it.productionId);
                if (it.dotPosition < prod.rhs.size()) {
                    symbolsAfterDot.insert(prod.rhs[it.dotPosition]);
                }
            }

            for (const auto& sym : symbolsAfterDot) {
                std::set<ParserLR1Item> g = gotoState(state, sym);
                if (g.empty()) continue;

                int nextState = findOrAddState(g);

                if (grammar.isTerminal(sym)) {
                    auto& entry = actionTable[static_cast<int>(si)][sym];

                    if (entry.type != ActionType::ERROR) {
                        std::cerr << "WARNING: Shift/Reduce conflict in state " << si
                                  << " on symbol " << sym.name << std::endl;
                    }

                    entry = Action(ActionType::SHIFT, nextState);
                } else {
                    gotoTable[static_cast<int>(si)][sym] = nextState;
                }
            }

            for (const auto& it : state) {
                if (it.productionId >= grammar.getProductionCount()) {
                    continue;
                }

                const auto& prod = grammar.getProduction(it.productionId);
                
                if (it.dotPosition == prod.rhs.size()) {
                    if (it.productionId == 0 && prod.lhs == grammar.getStartSymbol()) {
                        if (it.lookahead.name == "$") {
                            actionTable[static_cast<int>(si)][it.lookahead] =
                                Action(ActionType::ACCEPT, 0);
                        }
                    } else {
                        auto& entry = actionTable[static_cast<int>(si)][it.lookahead];

                        if (entry.type == ActionType::REDUCE) {
                            std::cerr << "WARNING: Reduce/Reduce conflict in state " << si
                                      << " on lookahead " << it.lookahead.name << std::endl;
                        }
                        else if (entry.type == ActionType::SHIFT) {
                            std::cerr << "WARNING: Shift/Reduce conflict in state " << si
                                      << " on lookahead " << it.lookahead.name
                                      << " (preferring shift)" << std::endl;
                            continue;
                        }

                        entry = Action(ActionType::REDUCE, it.productionId);
                    }
                }
            }
        }

        std::cout << "✓ Generated " << states.size() << " states" << std::endl;
        std::cout << "✓ ACTION table entries: " << actionTable.size() << std::endl;
        std::cout << "✓ GOTO table entries: " << gotoTable.size() << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "EXCEPTION in buildParsingTable: " << ex.what() << std::endl;
        throw;
    }

    std::cout << "=================================\n" << std::endl;
}

std::set<LR1Parser::ParserLR1Item> LR1Parser::closure(const std::set<ParserLR1Item>& items) {
    std::set<ParserLR1Item> result = items;
    bool changed = true;
    int iterations = 0;

    while (changed && iterations < 100) {
        changed = false;
        iterations++;
        std::set<ParserLR1Item> toAdd;

        for (const auto& item : result) {
            if (item.productionId >= grammar.getProductionCount()) {
                continue;
            }

            const auto& prod = grammar.getProduction(item.productionId);

            if (item.dotPosition >= prod.rhs.size()) {
                continue;
            }

            const GrammarSymbol& nextSymbol = prod.rhs[item.dotPosition];

            // std::cout << "[closure] Checking symbol: " << nextSymbol.name 
            //          << " isNonTerminal? " << grammar.isNonTerminal(nextSymbol) << std::endl;

            if (!grammar.isNonTerminal(nextSymbol)) {
                continue;
            }

            const auto& allProds = grammar.getProductions();
            for (size_t prodIdx = 0; prodIdx < allProds.size(); ++prodIdx) {
                const auto& newProd = allProds[prodIdx];

                if (newProd.lhs == nextSymbol) {
                    std::vector<GrammarSymbol> beta;
                    if (item.dotPosition + 1 < prod.rhs.size()) {
                        beta.insert(beta.end(),
                                   prod.rhs.begin() + item.dotPosition + 1,
                                   prod.rhs.end());
                    }
                    beta.push_back(item.lookahead);

                    auto firstSet = grammar.getFirst(beta);
                    
                    // std::cout << "   FIRST(" << nextSymbol.name << ") has " 
                    //           << firstSet.size() << " items" << std::endl;

                    for (const auto& lookahead : firstSet) {
                        if (lookahead.name == "ε" || lookahead.name == "epsilon") {
                            continue;
                        }

                        ParserLR1Item newItem(static_cast<int>(prodIdx), 0, lookahead);

                        if (result.find(newItem) == result.end() && 
                            toAdd.find(newItem) == toAdd.end()) {
                            toAdd.insert(newItem);
                            changed = true;
                            // std::cout << "     + Added item for production " << prodIdx << std::endl;
                        }
                    }
                }
            }
        }

        result.insert(toAdd.begin(), toAdd.end());
    }

    // std::cout << "[closure] Final result: " << result.size() << " items" << std::endl;
    return result;
}

std::set<LR1Parser::ParserLR1Item> LR1Parser::gotoState(const std::set<ParserLR1Item>& items, const GrammarSymbol& symbol) {
    std::set<ParserLR1Item> moved;

    // std::cout << "    [gotoState] Looking for symbol: '" << symbol.name 
    //           << "' (type: " << static_cast<int>(symbol.tokenType) << ")" << std::endl;

    for (const auto& item : items) {
        if (item.productionId >= grammar.getProductionCount()) continue;

        const auto& prod = grammar.getProduction(item.productionId);

        if (item.dotPosition < prod.rhs.size()) {
            const GrammarSymbol& itemSym = prod.rhs[item.dotPosition];
            
            // bool match = (itemSym == symbol);
            // std::cout << "      Item symbol: '" << itemSym.name 
            //           << "' (type: " << static_cast<int>(itemSym.tokenType) 
            //           << ") == '" << symbol.name << "'? " << match << std::endl;
            
            if (itemSym == symbol) {
                moved.insert(ParserLR1Item(item.productionId, item.dotPosition + 1, item.lookahead));
                // std::cout << "         ✓ MATCH! Moving dot forward." << std::endl;
            }
        }
    }
    
    // std::cout << "    [gotoState] Found " << moved.size() << " items to move" << std::endl;
    return closure(moved);
}

int LR1Parser::findOrAddState(const std::set<ParserLR1Item>& state) {
    for (size_t i = 0; i < states.size(); ++i) {
        if (states[i] == state) return static_cast<int>(i);
    }
    states.push_back(state);
    return static_cast<int>(states.size() - 1);
}

// LR1Parser.cpp — replace tokenToGrammarSymbol implementation with this:
GrammarSymbol LR1Parser::tokenToGrammarSymbol(const Token& token) const {
    switch (token.type) {
        // Keywords / type keywords
        case TokenType::VAR:          return GrammarSymbol("VAR", TokenType::VAR);
        case TokenType::INT:          return GrammarSymbol("INT", TokenType::INT);
        case TokenType::FLOAT_KW:     return GrammarSymbol("FLOAT", TokenType::FLOAT_KW);
        case TokenType::STRING_KW:    return GrammarSymbol("STRING", TokenType::STRING_KW);
        case TokenType::BOOL:         return GrammarSymbol("BOOL", TokenType::BOOL);
        case TokenType::VOID:         return GrammarSymbol("VOID", TokenType::VOID);

        // Control / function keywords
        case TokenType::IF:           return GrammarSymbol("IF", TokenType::IF);
        case TokenType::ELSE:         return GrammarSymbol("ELSE", TokenType::ELSE);
        case TokenType::WHILE:        return GrammarSymbol("WHILE", TokenType::WHILE);
        case TokenType::FOR:          return GrammarSymbol("FOR", TokenType::FOR);
        case TokenType::RETURN:       return GrammarSymbol("RETURN", TokenType::RETURN);
        case TokenType::FUNCTION:     return GrammarSymbol("FUNCTION", TokenType::FUNCTION);
        case TokenType::CONST:        return GrammarSymbol("CONST", TokenType::CONST);

        // Boolean literals — make names match grammar (TRUE_LIT / FALSE_LIT)
        case TokenType::TRUE:         return GrammarSymbol("TRUE_LIT", TokenType::TRUE);
        case TokenType::FALSE:        return GrammarSymbol("FALSE_LIT", TokenType::FALSE);

        // Identifiers / literals
        case TokenType::IDENTIFIER:   return GrammarSymbol("IDENTIFIER", TokenType::IDENTIFIER);
        case TokenType::INTEGER:      return GrammarSymbol("INTEGER", TokenType::INTEGER);
        case TokenType::FLOAT:        return GrammarSymbol("FLOAT_VAL", TokenType::FLOAT);
        case TokenType::STRING:       return GrammarSymbol("STRING_VAL", TokenType::STRING);
        case TokenType::BOOLEAN:      return GrammarSymbol("BOOLEAN", TokenType::BOOLEAN);

        // Operators
        case TokenType::ASSIGN:       return GrammarSymbol("ASSIGN", TokenType::ASSIGN);
        case TokenType::PLUS:         return GrammarSymbol("PLUS", TokenType::PLUS);
        case TokenType::MINUS:        return GrammarSymbol("MINUS", TokenType::MINUS);
        case TokenType::MULTIPLY:     return GrammarSymbol("MULTIPLY", TokenType::MULTIPLY);
        case TokenType::DIVIDE:       return GrammarSymbol("DIVIDE", TokenType::DIVIDE);
        case TokenType::MODULO:       return GrammarSymbol("MODULO", TokenType::MODULO);
        case TokenType::EQUAL:        return GrammarSymbol("EQUAL", TokenType::EQUAL);
        case TokenType::NOT_EQUAL:    return GrammarSymbol("NOT_EQUAL", TokenType::NOT_EQUAL);
        case TokenType::LESS:         return GrammarSymbol("LESS", TokenType::LESS);
        case TokenType::LESS_EQUAL:   return GrammarSymbol("LESS_EQUAL", TokenType::LESS_EQUAL);
        case TokenType::GREATER:      return GrammarSymbol("GREATER", TokenType::GREATER);
        case TokenType::GREATER_EQUAL:return GrammarSymbol("GREATER_EQUAL", TokenType::GREATER_EQUAL);
        case TokenType::LOGICAL_AND:  return GrammarSymbol("AND", TokenType::LOGICAL_AND);
        case TokenType::LOGICAL_OR:   return GrammarSymbol("OR", TokenType::LOGICAL_OR);
        case TokenType::LOGICAL_NOT:  return GrammarSymbol("NOT", TokenType::LOGICAL_NOT);

        // Punctuation
        case TokenType::SEMICOLON:    return GrammarSymbol("SEMICOLON", TokenType::SEMICOLON);
        case TokenType::COMMA:        return GrammarSymbol("COMMA", TokenType::COMMA);
        case TokenType::DOT:          return GrammarSymbol("DOT", TokenType::DOT);
        case TokenType::LEFT_PAREN:   return GrammarSymbol("LPAREN", TokenType::LEFT_PAREN);
        case TokenType::RIGHT_PAREN:  return GrammarSymbol("RPAREN", TokenType::RIGHT_PAREN);
        case TokenType::LEFT_BRACE:   return GrammarSymbol("LBRACE", TokenType::LEFT_BRACE);
        case TokenType::RIGHT_BRACE:  return GrammarSymbol("RBRACE", TokenType::RIGHT_BRACE);
        case TokenType::LEFT_BRACKET: return GrammarSymbol("LBRACKET", TokenType::LEFT_BRACKET);
        case TokenType::RIGHT_BRACKET:return GrammarSymbol("RBRACKET", TokenType::RIGHT_BRACKET);

        // EOF / others
        case TokenType::EOF_TOKEN:    return GrammarSymbol("$", TokenType::EOF_TOKEN);
        case TokenType::NEWLINE:      return GrammarSymbol("NEWLINE", TokenType::NEWLINE);

        default:
            return GrammarSymbol("ERROR", TokenType::ERROR_TOKEN);
    }
}

std::shared_ptr<ASTNode> LR1Parser::buildAST(const std::vector<std::shared_ptr<ASTNode>>& children, int productionId) {
    const auto& production = grammar.getProduction(productionId);

    if (production.lhs.name == "Program") {
        auto node = std::make_shared<ASTNode>(ASTNodeType::PROGRAM);
        node->children = children;
        return node;
    } else if (production.lhs.name == "VarDecl") {
        auto node = std::make_shared<ASTNode>(ASTNodeType::VARIABLE_DECLARATION);
        node->children = children;
        return node;
    } else if (production.lhs.name == "Expr" || production.lhs.name == "Term") {
        if (children.size() == 3) {
            auto node = std::make_shared<ASTNode>(ASTNodeType::BINARY_OPERATION, children[1]->value);
            node->children.push_back(children[0]);
            node->children.push_back(children[2]);
            return node;
        }
        if (!children.empty()) return children[0];
    } else if (production.lhs.name == "Factor") {
        if (children.size() == 1) return children[0];
        if (children.size() == 3) return children[1];
    }

    if (!children.empty()) return children[0];
    return std::make_shared<ASTNode>(ASTNodeType::EMPTY);
}

} // namespace SCERSE
