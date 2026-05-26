#pragma once

#include <string>

#include "IR.h"
#include "SymbolTable.h"

class CodeGenerator {
public:
    explicit CodeGenerator(const IRProgram& program, const SymbolTable& symTable);

    std::string generate() const;

private:
    void emitLine(std::string line) const;
    void emitDeclarations() const;
    void emitInstruction(const IRInstr& instr) const;
    bool isTemp(const std::string& name) const;

    const IRProgram& m_program;
    const SymbolTable& m_symTable;
    mutable std::string m_output;
};
