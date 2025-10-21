#include "LR1Parser.hpp"
#include <algorithm>
#include <iostream>

namespace SCERSE {

LR1Parser::LR1Parser() {
    // Make sure grammar is populated before table build.
    // If Grammar has a build() method, call it here; otherwise ensure Grammar ctor populates rules.
    // e.g. grammar.build();  // uncomment if you implement Grammar::build()
    try {
        buildParsingTable();
    } catch (const std::exception& ex) {
        std::cerr << "Warning: building LR(1) parsing table failed: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Warning: building LR(1) parsing table failed (unknown error)." << std::endl;
    }
}

ParseResult LR1Parser::parse(const std::vector<Token>& tokensIn) {
    ParseResult result;
    result.success = true;
    
    // Basic token validation - but DON'T return early, collect all errors
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
    
    // If states table was not built, return with collected errors (fallback)
    if (states.empty()) {
        std::cerr << "Warning: Parser states table is empty - skipping syntax analysis\n";
        return result;
    }
    
    // LR(1) parsing with error recovery
    std::stack<int> stateStack;
    std::stack<std::shared_ptr<ASTNode>> nodeStack;
    stateStack.push(0);
    
    // Make a copy of tokens and ensure EOF present
    std::vector<Token> tokens = tokensIn;
    if (tokens.empty() || tokens.back().type != TokenType::EOF_TOKEN) {
        tokens.push_back(Token{TokenType::EOF_TOKEN, "$", Position()});
    }
    
    size_t idx = 0;
    int errorCount = 0;
    const int MAX_ERRORS = 50; // Prevent infinite error loops
    
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
        
        // Find action
        auto stIt = actionTable.find(curState);
        if (stIt == actionTable.end()) {
            result.success = false;
            result.errors.push_back(
                CompilerError(ErrorSeverity::ERROR,
                             "Syntax error near: " + curToken.lexeme,
                             Position(curToken.position.line, curToken.position.column))
            );
            
            // ERROR RECOVERY: Skip this token and continue
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
            
            // ERROR RECOVERY: Skip this token and continue
            ++idx;
            ++errorCount;
            continue;
        }
        
        Action action = actIt->second;
        
        switch (action.type) {
            case ActionType::SHIFT: {
                stateStack.push(action.value);
                // create leaf node for token
                auto node = std::make_shared<ASTNode>(ASTNodeType::LITERAL, curToken.lexeme);
                node->position = SourcePosition(curToken.position.line, curToken.position.column);
                nodeStack.push(node);
                ++idx;
                break;
            }
            
            case ActionType::REDUCE: {
                const auto& prod = grammar.getProduction(action.value);
                std::vector<std::shared_ptr<ASTNode>> children;
                
                // Pop children & states
                for (size_t i = 0; i < prod.rhs.size(); ++i) {
                    if (!nodeStack.empty()) {
                        children.insert(children.begin(), nodeStack.top());
                        nodeStack.pop();
                    }
                    if (!stateStack.empty()) stateStack.pop();
                }
                
                // Build AST node for this production
                auto node = buildAST(children, action.value);
                nodeStack.push(node);
                
                // GOTO
                if (!stateStack.empty()) {
                    int topState = stateStack.top();
                    auto gotoIt = gotoTable.find(topState);
                    if (gotoIt != gotoTable.end()) {
                        auto symbolIt = gotoIt->second.find(prod.lhs);
                        if (symbolIt != gotoIt->second.end()) {
                            stateStack.push(symbolIt->second);
                        } else {
                            // missing goto -- parser table incomplete
                            result.success = false;
                            result.errors.push_back(
                                CompilerError(ErrorSeverity::ERROR,
                                             "Parser table missing GOTO entry during reduce",
                                             Position())
                            );
                            // Don't return - try to continue
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
                        // Don't return - try to continue
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
                // ERROR RECOVERY: Skip token and continue
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

    // Start production should be present at productions[0] with augmented start if applicable.
    // initial item: [S' -> · S, $] — we assume production 0 exists and startSymbol is available
    ParserLR1Item startItem(0, 0, GrammarSymbol("$", TokenType::EOF_TOKEN));
    std::set<ParserLR1Item> startSet = closure({startItem});
    states.push_back(startSet);

    // Build canonical collection (BFS style)
    for (size_t si = 0; si < states.size(); ++si) {
        const auto& state = states[si];

        // collect symbols that can appear after a dot in this state
        std::set<GrammarSymbol> symbolsAfterDot;
        for (const auto& it : state) {
            const auto& prod = grammar.getProduction(it.productionId);
            if (it.dotPosition < prod.rhs.size()) {
                symbolsAfterDot.insert(prod.rhs[it.dotPosition]);
            }
        }

        // For each symbol, compute goto(state, symbol)
        for (const auto& sym : symbolsAfterDot) {
            std::set<ParserLR1Item> g = gotoState(state, sym);
            if (g.empty()) continue;

            int nextState = findOrAddState(g);
            // If the grammar symbol is terminal => SHIFT on that symbol to nextState
            if (grammar.isTerminal(sym)) {
                actionTable[static_cast<int>(si)][sym] = Action(ActionType::SHIFT, nextState);
            } else {
                // Non-terminal => GOTO
                gotoTable[static_cast<int>(si)][sym] = nextState;
            }
        }

        // Add REDUCE/ACCEPT actions: for items with dot at end
        for (const auto& it : state) {
            const auto& prod = grammar.getProduction(it.productionId);
            if (it.dotPosition == prod.rhs.size()) {
                // Completed item [A -> α ·, a]
                if (prod.lhs == grammar.getStartSymbol()) {
                    // If production is start -> S, then on lookahead accept
                    actionTable[static_cast<int>(si)][it.lookahead] = Action(ActionType::ACCEPT, 0);
                } else {
                    // On lookahead a, reduce by production
                    actionTable[static_cast<int>(si)][it.lookahead] = Action(ActionType::REDUCE, it.productionId);
                }
            }
        }
    }

    // Note: This simple construction may produce shift/reduce conflicts if grammar is ambiguous.
}

std::set<LR1Parser::ParserLR1Item> LR1Parser::closure(const std::set<ParserLR1Item>& items) {
    std::set<ParserLR1Item> result = items;
    bool changed = true;

    while (changed) {
        changed = false;
        std::set<ParserLR1Item> toAdd;

        for (const auto& it : result) {
            const auto& prod = grammar.getProduction(it.productionId);
            if (it.dotPosition >= prod.rhs.size()) continue;

            const GrammarSymbol& B = prod.rhs[it.dotPosition];
            if (!grammar.isNonTerminal(B)) continue;

            // compute FIRST(beta + lookahead)
            std::vector<GrammarSymbol> beta;
            if (it.dotPosition + 1 < prod.rhs.size()) {
                beta.insert(beta.end(), prod.rhs.begin() + it.dotPosition + 1, prod.rhs.end());
            }
            beta.push_back(it.lookahead);

            auto firstBeta = grammar.getFirst(beta); // set<GrammarSymbol>

            // for each production B -> gamma
            const auto& allProds = grammar.getProductions();
            for (size_t p = 0; p < allProds.size(); ++p) {
                if (allProds[p].lhs == B) {
                    for (const auto& a : firstBeta) {
                        if (a.name == "ε") continue;
                        ParserLR1Item newItem(static_cast<int>(p), 0, a);
                        if (result.find(newItem) == result.end() && toAdd.find(newItem) == toAdd.end()) {
                            toAdd.insert(newItem);
                            changed = true;
                        }
                    }
                }
            }
        }

        result.insert(toAdd.begin(), toAdd.end());
    }

    return result;
}

std::set<LR1Parser::ParserLR1Item> LR1Parser::gotoState(const std::set<ParserLR1Item>& items, const GrammarSymbol& symbol) {
    std::set<ParserLR1Item> moved;
    for (const auto& it : items) {
        const auto& prod = grammar.getProduction(it.productionId);
        if (it.dotPosition < prod.rhs.size() && prod.rhs[it.dotPosition] == symbol) {
            moved.insert(ParserLR1Item(it.productionId, it.dotPosition + 1, it.lookahead));
        }
    }
    return closure(moved);
}

int LR1Parser::findOrAddState(const std::set<ParserLR1Item>& state) {
    for (size_t i = 0; i < states.size(); ++i) {
        if (states[i] == state) return static_cast<int>(i);
    }
    states.push_back(state);
    return static_cast<int>(states.size() - 1);
}

GrammarSymbol LR1Parser::tokenToGrammarSymbol(const Token& token) const {
    switch (token.type) {
        // Keywords
        case TokenType::VAR: return GrammarSymbol("VAR", TokenType::VAR);
        case TokenType::INT: return GrammarSymbol("INT", TokenType::INT);
        case TokenType::FLOAT_KW: return GrammarSymbol("FLOAT", TokenType::FLOAT_KW);
        case TokenType::STRING_KW: return GrammarSymbol("STRING", TokenType::STRING_KW);
        case TokenType::BOOL: return GrammarSymbol("BOOL", TokenType::BOOL);
        case TokenType::VOID: return GrammarSymbol("VOID", TokenType::VOID);
        case TokenType::IF: return GrammarSymbol("IF", TokenType::IF);
        case TokenType::ELSE: return GrammarSymbol("ELSE", TokenType::ELSE);
        case TokenType::WHILE: return GrammarSymbol("WHILE", TokenType::WHILE);
        case TokenType::FOR: return GrammarSymbol("FOR", TokenType::FOR);
        case TokenType::RETURN: return GrammarSymbol("RETURN", TokenType::RETURN);
        case TokenType::FUNCTION: return GrammarSymbol("FUNCTION", TokenType::FUNCTION);
        case TokenType::CONST: return GrammarSymbol("CONST", TokenType::CONST);
        case TokenType::TRUE: return GrammarSymbol("TRUE", TokenType::TRUE);
        case TokenType::FALSE: return GrammarSymbol("FALSE", TokenType::FALSE);

        // Identifiers and Literals
        case TokenType::IDENTIFIER: return GrammarSymbol("IDENTIFIER", TokenType::IDENTIFIER);
        case TokenType::INTEGER: return GrammarSymbol("INTEGER", TokenType::INTEGER);
        case TokenType::FLOAT: return GrammarSymbol("FLOAT_VAL", TokenType::FLOAT);
        case TokenType::STRING: return GrammarSymbol("STRING_VAL", TokenType::STRING);
        case TokenType::BOOLEAN: return GrammarSymbol("BOOLEAN", TokenType::BOOLEAN);

        // Operators
        case TokenType::ASSIGN: return GrammarSymbol("ASSIGN", TokenType::ASSIGN);
        case TokenType::PLUS: return GrammarSymbol("PLUS", TokenType::PLUS);
        case TokenType::MINUS: return GrammarSymbol("MINUS", TokenType::MINUS);
        case TokenType::MULTIPLY: return GrammarSymbol("MULTIPLY", TokenType::MULTIPLY);
        case TokenType::DIVIDE: return GrammarSymbol("DIVIDE", TokenType::DIVIDE);
        case TokenType::MODULO: return GrammarSymbol("MODULO", TokenType::MODULO);
        
        // Comparison Operators
        case TokenType::EQUAL: return GrammarSymbol("EQUAL", TokenType::EQUAL);
        case TokenType::NOT_EQUAL: return GrammarSymbol("NOT_EQUAL", TokenType::NOT_EQUAL);
        case TokenType::LESS: return GrammarSymbol("LESS", TokenType::LESS);
        case TokenType::LESS_EQUAL: return GrammarSymbol("LESS_EQUAL", TokenType::LESS_EQUAL);
        case TokenType::GREATER: return GrammarSymbol("GREATER", TokenType::GREATER);
        case TokenType::GREATER_EQUAL: return GrammarSymbol("GREATER_EQUAL", TokenType::GREATER_EQUAL);
        
        // Logical Operators
        case TokenType::LOGICAL_AND: return GrammarSymbol("AND", TokenType::LOGICAL_AND);
        case TokenType::LOGICAL_OR: return GrammarSymbol("OR", TokenType::LOGICAL_OR);
        case TokenType::LOGICAL_NOT: return GrammarSymbol("NOT", TokenType::LOGICAL_NOT);

        // Delimiters
        case TokenType::SEMICOLON: return GrammarSymbol("SEMICOLON", TokenType::SEMICOLON);
        case TokenType::COMMA: return GrammarSymbol("COMMA", TokenType::COMMA);
        case TokenType::DOT: return GrammarSymbol("DOT", TokenType::DOT);
        case TokenType::LEFT_PAREN: return GrammarSymbol("LPAREN", TokenType::LEFT_PAREN);
        case TokenType::RIGHT_PAREN: return GrammarSymbol("RPAREN", TokenType::RIGHT_PAREN);
        case TokenType::LEFT_BRACE: return GrammarSymbol("LBRACE", TokenType::LEFT_BRACE);
        case TokenType::RIGHT_BRACE: return GrammarSymbol("RBRACE", TokenType::RIGHT_BRACE);
        case TokenType::LEFT_BRACKET: return GrammarSymbol("LBRACKET", TokenType::LEFT_BRACKET);
        case TokenType::RIGHT_BRACKET: return GrammarSymbol("RBRACKET", TokenType::RIGHT_BRACKET);

        // Special
        case TokenType::EOF_TOKEN: return GrammarSymbol("$", TokenType::EOF_TOKEN);
        case TokenType::NEWLINE: return GrammarSymbol("NEWLINE", TokenType::NEWLINE);
        
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
