#pragma once
#include "Types.hpp"
#include <string>
#include <vector>
namespace SCERSE {

class Position {
public:
    int line, column;
    Position(int l=1, int c=1) : line(l), column(c) {}
};

class CompilerError {
public:
    ErrorSeverity severity;
    std::string message;
    Position position;
    CompilerError(ErrorSeverity sev, const std::string& msg, const Position& pos)
        : severity(sev), message(msg), position(pos) {}
};

class ErrorReporter {
private:
    std::vector<CompilerError> errors;
public:
    void reportError(ErrorSeverity sev, const std::string& msg, const Position& pos) {
        errors.emplace_back(sev, msg, pos);
    }
    bool hasErrors() const { return !errors.empty(); }
    const std::vector<CompilerError>& getErrors() const { return errors; }
};

}
