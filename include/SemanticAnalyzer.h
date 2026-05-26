#pragma once

#include "AST.h"
#include "SymbolTable.h"

struct ExprTypeInfo {
    VarType type;
    int rows = 0;
    int cols = 0;
    bool isConstant = false;
    int constValue = 0;
};

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(SymbolTable& symTable);

    void analyze(Program& program);

    const SymbolTable& symbolTable() const { return m_symTable; }

private:
    void analyzeStmt(Statement& stmt);
    ExprTypeInfo analyzeExpr(const ASTNode& node);

    void visitMatrixDecl(MatrixDecl& stmt);
    void visitAssignStmt(AssignStmt& stmt);
    void visitPrintStmt(PrintStmt& stmt);

    SymbolTable& m_symTable;
};
