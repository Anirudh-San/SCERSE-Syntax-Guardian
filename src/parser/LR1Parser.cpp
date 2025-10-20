#include "LR1Parser.hpp"
#include <algorithm>
#include <iostream>

namespace SCERSE {

LR1Parser::LR1Parser() {
    // Build parsing table on construction
    try {
        buildParsingTable();
    } catch (...) {
        // If table building fails, parser will fall back to error reporting
        std::cerr << "Warning: Failed to build LR(1) parsing table" << std::endl;
    }
}

ParseResult LR1Parser::parse(const std::vector<Token>& tokens) {
    ParseResult result;
    result.success = true;
    
    // Early validation: check for error tokens (your existing logic)
    for (const auto& token : tokens) {
        if (token.type == TokenType::ERROR_TOKEN) {
            result.success = false;
            
            // Use your CompilerError constructor: CompilerError(severity, message, position)
            result.errors.push_back(
                CompilerError(
                    ErrorSeverity::ERROR,
                    "Unexpected or unknown token: " + token.lexeme,
                    Position(token.position.line, token.position.column)
                )
            );
        }
    }
    
    // Check for EOF (your existing logic)
    if (!tokens.empty() && tokens.back().type != TokenType::EOF_TOKEN) {
        result.success = false;
        
        result.errors.push_back(
            CompilerError(
                ErrorSeverity::ERROR,
                "Missing semicolon or end of statement",
                Position(tokens.back().position.line, tokens.back().position.column)
            )
        );
    }
    
    // If basic validation failed, return early
    if (!result.success) {
        return result;
    }
    
    // If parsing table is empty, return with basic validation only
    if (states.empty()) {
        return result;
    }
    
    // === LR(1) PARSING STARTS HERE ===
    
    std::stack<int> stateStack;
    std::stack<std::shared_ptr<ASTNode>> nodeStack;
    
    stateStack.push(0);
    
    size_t tokenIndex = 0;
    
    while (true) {
        int currentState = stateStack.top();
        Token currentToken = (tokenIndex < tokens.size()) ? tokens[tokenIndex] : 
                    Token{TokenType::EOF_TOKEN, "$", Position()};
        
        Symbol currentSymbol = tokenToSymbol(currentToken);
        
        // Look up action
        auto stateIter = actionTable.find(currentState);
        if (stateIter == actionTable.end()) {
            result.success = false;
            
            result.errors.push_back(
                CompilerError(
                    ErrorSeverity::ERROR,
                    "Unexpected token: " + currentToken.lexeme,
                    Position(currentToken.position.line, currentToken.position.column)
                )
            );
            break;
        }
        
        auto actionIter = stateIter->second.find(currentSymbol);
        if (actionIter == stateIter->second.end()) {
            result.success = false;
            
            result.errors.push_back(
                CompilerError(
                    ErrorSeverity::ERROR,
                    "Unexpected token: " + currentToken.lexeme,
                    Position(currentToken.position.line, currentToken.position.column)
                )
            );
            break;
        }
        
        Action action = actionIter->second;
        
        switch (action.type) {
            case ActionType::SHIFT: {
                stateStack.push(action.value);
                
                // Create leaf AST node for terminal
                auto node = std::make_shared<ASTNode>(ASTNodeType::LITERAL, currentToken.lexeme);
                node->position = SourcePosition(currentToken.position.line, currentToken.position.column);
                nodeStack.push(node);
                
                tokenIndex++;
                break;
            }
            
            case ActionType::REDUCE: {
                const auto& production = grammar.getProduction(action.value);
                
                std::vector<std::shared_ptr<ASTNode>> children;
                for (size_t i = 0; i < production.rhs.size(); ++i) {
                    if (!nodeStack.empty()) {
                        children.insert(children.begin(), nodeStack.top());
                        nodeStack.pop();
                    }
                    if (!stateStack.empty()) {
                        stateStack.pop();
                    }
                }
                
                // Build AST node for this production
                auto node = buildAST(children, action.value);
                nodeStack.push(node);
                
                // Goto
                if (!stateStack.empty()) {
                    int topState = stateStack.top();
                    auto gotoIter = gotoTable.find(topState);
                    if (gotoIter != gotoTable.end()) {
                        auto symbolIter = gotoIter->second.find(production.lhs);
                        if (symbolIter != gotoIter->second.end()) {
                            stateStack.push(symbolIter->second);
                        }
                    }
                }
                break;
            }
            
            case ActionType::ACCEPT:
                if (!nodeStack.empty()) {
                    result.ast = nodeStack.top();
                }
                result.success = true;
                return result;
            
            case ActionType::ERROR:
            default: {
                result.success = false;
                
                result.errors.push_back(
                    CompilerError(
                        ErrorSeverity::ERROR,
                        "Parse error at token: " + currentToken.lexeme,
                        Position(currentToken.position.line, currentToken.position.column)
                    )
                );
                return result;
            }
        }
    }
    
    return result;
}

void LR1Parser::buildParsingTable() {
    // Create initial state: [S' -> ·S, $]
    std::set<LR1Item> initialItems;
    initialItems.insert(LR1Item(0, 0, Symbol("$", TokenType::EOF_TOKEN)));
    
    std::set<LR1Item> initialState = closure(initialItems);
    states.push_back(initialState);
    
    // Build all states
    for (size_t i = 0; i < states.size(); ++i) {
        const auto& state = states[i];
        
        // Collect all symbols after the dot
        std::set<Symbol> symbols;
        for (const auto& item : state) {
            const auto& production = grammar.getProduction(item.productionId);
            
            if (item.dotPosition < production.rhs.size()) {
                symbols.insert(production.rhs[item.dotPosition]);
            }
        }
        
        // For each symbol, compute goto state
        for (const auto& symbol : symbols) {
            std::set<LR1Item> gotoSet = gotoState(state, symbol);
            
            if (!gotoSet.empty()) {
                int nextState = findOrAddState(gotoSet);
                
                if (grammar.isTerminal(symbol)) {
                    // SHIFT action
                    actionTable[i][symbol] = Action(ActionType::SHIFT, nextState);
                } else {
                    // GOTO entry
                    gotoTable[i][symbol] = nextState;
                }
            }
        }
        
        // Add REDUCE actions
        for (const auto& item : state) {
            const auto& production = grammar.getProduction(item.productionId);
            
            if (item.dotPosition == production.rhs.size()) {
                // Item is complete: [A -> α·, a]
                if (production.lhs == grammar.getStartSymbol()) {
                    // ACCEPT action
                    actionTable[i][item.lookahead] = Action(ActionType::ACCEPT, 0);
                } else {
                    // REDUCE action
                    actionTable[i][item.lookahead] = Action(ActionType::REDUCE, item.productionId);
                }
            }
        }
    }
}

std::set<LR1Item> LR1Parser::closure(const std::set<LR1Item>& items) {
    std::set<LR1Item> result = items;
    bool changed = true;
    
    while (changed) {
        changed = false;
        std::set<LR1Item> newItems;
        
        for (const auto& item : result) {
            const auto& production = grammar.getProduction(item.productionId);
            
            if (item.dotPosition < production.rhs.size()) {
                const Symbol& nextSymbol = production.rhs[item.dotPosition];
                
                if (grammar.isNonTerminal(nextSymbol)) {
                    // Compute lookaheads
                    std::vector<Symbol> beta(production.rhs.begin() + item.dotPosition + 1, 
                                            production.rhs.end());
                    beta.push_back(item.lookahead);
                    
                    auto firstBeta = grammar.getFirst(beta);
                    
                    // Add items for all productions of nextSymbol
                    const auto& prods = grammar.getProductions();
                    for (size_t i = 0; i < prods.size(); ++i) {
                        if (prods[i].lhs == nextSymbol) {
                            for (const auto& lookahead : firstBeta) {
                                if (lookahead.name != "ε") {
                                    LR1Item newItem(i, 0, lookahead);
                                    if (result.find(newItem) == result.end()) {
                                        newItems.insert(newItem);
                                        changed = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        result.insert(newItems.begin(), newItems.end());
    }
    
    return result;
}

std::set<LR1Item> LR1Parser::gotoState(const std::set<LR1Item>& items, const Symbol& symbol) {
    std::set<LR1Item> result;
    
    for (const auto& item : items) {
        const auto& production = grammar.getProduction(item.productionId);
        
        if (item.dotPosition < production.rhs.size() && 
            production.rhs[item.dotPosition] == symbol) {
            result.insert(LR1Item(item.productionId, item.dotPosition + 1, item.lookahead));
        }
    }
    
    return closure(result);
}

int LR1Parser::findOrAddState(const std::set<LR1Item>& state) {
    for (size_t i = 0; i < states.size(); ++i) {
        if (states[i] == state) {
            return i;
        }
    }
    
    states.push_back(state);
    return states.size() - 1;
}

Symbol LR1Parser::tokenToSymbol(const Token& token) {
    switch (token.type) {
        // Keywords
        case TokenType::VAR:
            return Symbol("VAR", TokenType::VAR);
        case TokenType::INT:
            return Symbol("INT", TokenType::INT);
        case TokenType::FLOAT_KW:
            return Symbol("FLOAT", TokenType::FLOAT_KW);
        case TokenType::STRING_KW:
            return Symbol("STRING", TokenType::STRING_KW);
        case TokenType::BOOL:
            return Symbol("BOOL", TokenType::BOOL);
            
        // Identifiers and literals
        case TokenType::IDENTIFIER:
            return Symbol("IDENTIFIER", TokenType::IDENTIFIER);
        case TokenType::INTEGER:
            return Symbol("INTEGER", TokenType::INTEGER);
        case TokenType::FLOAT:
            return Symbol("FLOAT_VAL", TokenType::FLOAT);
        case TokenType::STRING:
            return Symbol("STRING_VAL", TokenType::STRING);
            
        // Operators
        case TokenType::ASSIGN:
            return Symbol("ASSIGN", TokenType::ASSIGN);
        case TokenType::PLUS:
            return Symbol("PLUS", TokenType::PLUS);
        case TokenType::MINUS:
            return Symbol("MINUS", TokenType::MINUS);
        case TokenType::MULTIPLY:
            return Symbol("MULTIPLY", TokenType::MULTIPLY);
        case TokenType::DIVIDE:
            return Symbol("DIVIDE", TokenType::DIVIDE);
            
        // Punctuation
        case TokenType::SEMICOLON:
            return Symbol("SEMICOLON", TokenType::SEMICOLON);
        case TokenType::LEFT_PAREN:
            return Symbol("LPAREN", TokenType::LEFT_PAREN);
        case TokenType::RIGHT_PAREN:
            return Symbol("RPAREN", TokenType::RIGHT_PAREN);
            
        // End of file
        case TokenType::EOF_TOKEN:
            return Symbol("$", TokenType::EOF_TOKEN);
            
        default:
            return Symbol("ERROR", TokenType::EOF_TOKEN);
    }
}

std::shared_ptr<ASTNode> LR1Parser::buildAST(const std::vector<std::shared_ptr<ASTNode>>& children, 
                                             int productionId) {
    const auto& production = grammar.getProduction(productionId);
    
    // Create AST node based on production
    if (production.lhs.name == "Program") {
        auto node = std::make_shared<ASTNode>(ASTNodeType::PROGRAM);
        node->children = children;
        return node;
    }
    else if (production.lhs.name == "VarDecl") {
        auto node = std::make_shared<ASTNode>(ASTNodeType::VARIABLE_DECLARATION);
        node->children = children;
        return node;
    }
    else if (production.lhs.name == "Expr" || production.lhs.name == "Term") {
        if (children.size() == 3) {
            // Binary operation
            auto node = std::make_shared<ASTNode>(ASTNodeType::BINARY_OPERATION, children[1]->value);
            node->children.push_back(children[0]);
            node->children.push_back(children[2]);
            return node;
        }
        if (!children.empty()) {
            return children[0];
        }
    }
    else if (production.lhs.name == "Factor") {
        if (children.size() == 1) {
            return children[0];
        } else if (children.size() == 3) {
            // Parenthesized expression
            return children[1];
        }
    }
    
    // Default: pass through children or create empty node
    if (!children.empty()) {
        return children[0];
    }
    return std::make_shared<ASTNode>(ASTNodeType::EMPTY);
}

} // namespace SCERSE
