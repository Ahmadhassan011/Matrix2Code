#pragma once

#include <string>
#include <unordered_map>

enum class VarType { SCALAR, MATRIX };

struct SymbolInfo {
    VarType type;
    int matrixRows;
    int matrixCols;
    int lineDeclared;
    bool initialized;
};

class SymbolTable {
public:
    void declare(const std::string& name, VarType type, int line,
                 int matrixRows = 0, int matrixCols = 0);
    void initialize(const std::string& name);
    bool isDeclared(const std::string& name) const;
    SymbolInfo& lookup(const std::string& name);
    size_t size() const { return m_symbols.size(); }

private:
    std::unordered_map<std::string, SymbolInfo> m_symbols;
};
