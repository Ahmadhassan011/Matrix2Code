#pragma once

#include <string>

#include "AST.h"
#include "IR.h"
#include "SymbolTable.h"

class IRGenerator {
public:
    IRGenerator(IRProgram& program, SymbolTable& symTable);
    void generate(const Program& ast);

private:
    struct ExprResult {
        std::string ref;
        VarType type;
        int rows;
        int cols;
    };

    void generateStmt(const Statement& stmt);
    void generateMatrixDecl(const MatrixDecl& stmt);
    void generateAssign(const AssignStmt& stmt);
    void generatePrint(const PrintStmt& stmt);

    ExprResult translateExpr(const ASTNode& node, const std::string& dstHint = "");

    IRProgram& m_program;
    SymbolTable& m_symTable;
};
