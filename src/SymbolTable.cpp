#include "SymbolTable.h"
#include "ErrorHandler.h"

void SymbolTable::declare(const std::string& name, VarType type, int line,
                          int matrixRows, int matrixCols) {
    if (m_symbols.find(name) != m_symbols.end()) {
        throw SemanticError(
            "'" + name + "' already declared",
            line, 1
        );
    }
    m_symbols[name] = {type, matrixRows, matrixCols, line, false};
}

void SymbolTable::initialize(const std::string& name) {
    auto it = m_symbols.find(name);
    if (it != m_symbols.end()) {
        it->second.initialized = true;
    }
}

bool SymbolTable::isDeclared(const std::string& name) const {
    return m_symbols.find(name) != m_symbols.end();
}

SymbolInfo& SymbolTable::lookup(const std::string& name) {
    auto it = m_symbols.find(name);
    if (it == m_symbols.end()) {
        throw SemanticError("'" + name + "' undeclared", 0, 1);
    }
    return it->second;
}
