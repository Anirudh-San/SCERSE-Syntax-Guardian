#include "SuggestionEngine.hpp"
#include <algorithm>
#include <cmath>

using namespace SCERSE;

std::vector<std::string> SuggestionEngine::generateSuggestions(
    const std::vector<CompilerError>& errors, const SymbolTable& symbolTable) {

    std::vector<std::string> suggestions;

    for (const auto& err : errors) {
        std::string suggestion;
        
        if (err.message.find("undeclared") != std::string::npos) {
            auto symbols = symbolTable.getAllSymbols();
            std::string closest;
            int best = 9999;
            for (const auto& sym : symbols) {
                int d = std::abs((int)err.message.size() - (int)sym.name.size());
                if (d < best) {
                    best = d;
                    closest = sym.name;
                }
            }
            suggestion = "[Line " + std::to_string(err.position.line) + "] Did you mean '" + closest + "'?";
        }
        else if (err.message.find("missing ;") != std::string::npos) {
            suggestion = "[Line " + std::to_string(err.position.line) + "] Add a semicolon ';' at the end of the statement.";
        }
        else {
            suggestion = "[Line " + std::to_string(err.position.line) + "] Check syntax near this line.";
        }
        
        suggestions.push_back(suggestion);
    }
    return suggestions;
}

