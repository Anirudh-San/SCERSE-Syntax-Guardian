#pragma once
#include "Token.hpp"
#include "../common/Error.hpp"
#include <string>
#include <vector>
namespace SCERSE {

class Lexer {
private:
    std::string source;
    size_t current = 0;
    Position position;
    ErrorReporter& errorReporter;
public:
    Lexer(const std::string& src, ErrorReporter& rep) : source(src), errorReporter(rep) {}
    Token nextToken();
    std::vector<Token> tokenize();
};

}
