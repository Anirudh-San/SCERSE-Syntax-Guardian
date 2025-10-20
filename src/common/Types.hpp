#pragma once

#include <string>
namespace SCERSE {

enum class DataType { INTEGER, FLOAT, BOOLEAN, STRING, VOID, UNKNOWN };
enum class ScopeType { GLOBAL, FUNCTION, BLOCK };
enum class SymbolType { VARIABLE, FUNCTION, PARAMETER, CONSTANT };
enum class ErrorSeverity { WARNING, ERROR, FATAL };
enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

inline const char* to_cstring(DataType dt) {
    switch (dt) {
        case DataType::INTEGER: return "int";
        case DataType::FLOAT:   return "float";
        case DataType::BOOLEAN: return "bool";
        case DataType::STRING:  return "string";
        case DataType::VOID:    return "void";
        default:                return "unknown";
    }
}
inline std::string to_string(DataType dt) { return std::string(to_cstring(dt)); }

}
