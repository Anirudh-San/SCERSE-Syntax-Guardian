#pragma once
#include "../common/Error.hpp"
#include "../semantic/SymbolTable.hpp"
#include <string>
#include <vector>

namespace SCERSE {
class SuggestionEngine {
public:
    std::vector<std::string> generateSuggestions(const std::vector<CompilerError>& errors,
                                                const SymbolTable& symbolTable);
};
}
