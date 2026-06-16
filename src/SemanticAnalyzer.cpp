#include "SemanticAnalyzer.h"
#include "ErrorHandler.h"

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& symTable)
    : m_symTable(symTable) {}

void SemanticAnalyzer::analyze(Program& program) {
    for (const auto& stmt : program.statements()) {
        analyzeStmt(*stmt);
    }
}

void SemanticAnalyzer::analyzeStmt(Statement& stmt) {
    if (auto* md = dynamic_cast<MatrixDecl*>(&stmt)) {
        visitMatrixDecl(*md);
    } else if (auto* as = dynamic_cast<AssignStmt*>(&stmt)) {
        visitAssignStmt(*as);
    } else if (auto* ps = dynamic_cast<PrintStmt*>(&stmt)) {
        visitPrintStmt(*ps);
    }
}

void SemanticAnalyzer::visitMatrixDecl(MatrixDecl& stmt) {
    m_symTable.declare(stmt.name(), VarType::MATRIX, stmt.line(),
                       stmt.literal()->rows(), stmt.literal()->cols());
    m_symTable.initialize(stmt.name());
}

void SemanticAnalyzer::visitAssignStmt(AssignStmt& stmt) {
    ExprTypeInfo exprType = analyzeExpr(*stmt.expr());

    if (!m_symTable.isDeclared(stmt.name())) {
        m_symTable.declare(stmt.name(), exprType.type, stmt.line(),
                           exprType.rows, exprType.cols);
    } else {
        SymbolInfo& sym = m_symTable.lookup(stmt.name());
        if (sym.type != exprType.type) {
            throw SemanticError(
                "cannot assign " + std::string(exprType.type == VarType::SCALAR ? "scalar" : "matrix") +
                " value to '" + stmt.name() + "' (declared as " +
                std::string(sym.type == VarType::SCALAR ? "scalar" : "matrix") + ")",
                stmt.line(), 1
            );
        }
        if (sym.type == VarType::MATRIX) {
            if (sym.matrixRows != exprType.rows || sym.matrixCols != exprType.cols) {
                throw SemanticError(
                    "cannot assign " + std::to_string(exprType.rows) + "x" +
                    std::to_string(exprType.cols) + " matrix to '" +
                    stmt.name() + "' (declared " + std::to_string(sym.matrixRows) +
                    "x" + std::to_string(sym.matrixCols) + ")",
                    stmt.line(), 1
                );
            }
        }
    }

    m_symTable.initialize(stmt.name());

    if (exprType.isConstant && exprType.type == VarType::SCALAR) {
        SymbolInfo& sym = m_symTable.lookup(stmt.name());
        sym.initialized = true;
    }
}

void SemanticAnalyzer::visitPrintStmt(PrintStmt& stmt) {
    if (!m_symTable.isDeclared(stmt.name())) {
        throw SemanticError("'" + stmt.name() + "' undeclared", stmt.line(), 1);
    }
    SymbolInfo& sym = m_symTable.lookup(stmt.name());
    if (!sym.initialized) {
        throw SemanticError("'" + stmt.name() + "' may be uninitialized", stmt.line(), 1);
    }
}

ExprTypeInfo SemanticAnalyzer::analyzeExpr(const ASTNode& node) {
    if (auto* num = dynamic_cast<const NumberLiteral*>(&node)) {
        return {VarType::SCALAR, 0, 0, true, num->value()};
    }

    if (auto* ident = dynamic_cast<const Identifier*>(&node)) {
        SymbolInfo& sym = m_symTable.lookup(ident->name());
        return {sym.type, sym.matrixRows, sym.matrixCols, false, 0};
    }

    if (auto* binop = dynamic_cast<const BinaryOp*>(&node)) {
        ExprTypeInfo left = analyzeExpr(*binop->left());
        ExprTypeInfo right = analyzeExpr(*binop->right());

        if (binop->op() == BinaryOp::ADD || binop->op() == BinaryOp::SUB) {
            if (left.type == VarType::SCALAR && right.type == VarType::SCALAR) {
                int val = (binop->op() == BinaryOp::ADD)
                    ? left.constValue + right.constValue
                    : left.constValue - right.constValue;
                return {VarType::SCALAR, 0, 0,
                        left.isConstant && right.isConstant, val};
            }
            if (left.type == VarType::MATRIX && right.type == VarType::MATRIX) {
                if (left.rows != right.rows || left.cols != right.cols) {
                    throw SemanticError(
                        "matrix dimensions (" + std::to_string(left.rows) + "x" +
                        std::to_string(left.cols) + ") and (" +
                        std::to_string(right.rows) + "x" +
                        std::to_string(right.cols) +
                        ") incompatible for addition/subtraction",
                        binop->line(), 1
                    );
                }
                return {VarType::MATRIX, left.rows, left.cols, false, 0};
            }
            throw SemanticError(
                "cannot add/subtract " +
                std::string(left.type == VarType::SCALAR ? "scalar" : "matrix") +
                " and " +
                std::string(right.type == VarType::SCALAR ? "scalar" : "matrix"),
                binop->line(), 1
            );
        }

        if (binop->op() == BinaryOp::MUL) {
            if (left.type == VarType::SCALAR && right.type == VarType::SCALAR) {
                int val = left.constValue * right.constValue;
                return {VarType::SCALAR, 0, 0,
                        left.isConstant && right.isConstant, val};
            }
            if (left.type == VarType::SCALAR && right.type == VarType::MATRIX) {
                return {VarType::MATRIX, right.rows, right.cols, false, 0};
            }
            if (left.type == VarType::MATRIX && right.type == VarType::SCALAR) {
                return {VarType::MATRIX, left.rows, left.cols, false, 0};
            }
            if (left.type == VarType::MATRIX && right.type == VarType::MATRIX) {
                if (left.cols != right.rows) {
                    throw SemanticError(
                        "matrix dimensions (" + std::to_string(left.rows) + "x" +
                        std::to_string(left.cols) + ") and (" +
                        std::to_string(right.rows) + "x" +
                        std::to_string(right.cols) +
                        ") incompatible for multiplication",
                        binop->line(), 1
                    );
                }
                return {VarType::MATRIX, left.rows, right.cols, false, 0};
            }
            throw SemanticError(
                "cannot multiply " +
                std::string(left.type == VarType::SCALAR ? "scalar" : "matrix") +
                " and " +
                std::string(right.type == VarType::SCALAR ? "scalar" : "matrix"),
                binop->line(), 1
            );
        }

        if (binop->op() == BinaryOp::DIV) {
            if (left.type == VarType::SCALAR && right.type == VarType::SCALAR) {
                if (right.isConstant && right.constValue == 0) {
                    throw SemanticError("division by zero", binop->line(), 1);
                }
                int val = (right.isConstant && right.constValue != 0)
                    ? left.constValue / right.constValue : 0;
                return {VarType::SCALAR, 0, 0,
                        left.isConstant && right.isConstant, val};
            }
            throw SemanticError(
                "cannot divide " +
                std::string(left.type == VarType::SCALAR ? "scalar" : "matrix") +
                " and " +
                std::string(right.type == VarType::SCALAR ? "scalar" : "matrix"),
                binop->line(), 1
            );
        }
    }

    if (auto* te = dynamic_cast<const TransposeExpr*>(&node)) {
        ExprTypeInfo inner = analyzeExpr(*te->expr());
        if (inner.type != VarType::MATRIX) {
            throw SemanticError("cannot transpose a scalar", te->line(), 1);
        }
        return {VarType::MATRIX, inner.cols, inner.rows, false, 0};
    }

    if (auto* de = dynamic_cast<const DetExpr*>(&node)) {
        ExprTypeInfo inner = analyzeExpr(*de->expr());
        if (inner.type != VarType::MATRIX) {
            throw SemanticError("det requires a matrix argument", de->line(), 1);
        }
        if (inner.rows != inner.cols) {
            throw SemanticError(
                "det requires a square matrix, got " +
                std::to_string(inner.rows) + "x" +
                std::to_string(inner.cols),
                de->line(), 1
            );
        }
        return {VarType::SCALAR, 0, 0, false, 0};
    }

    if (auto* mat = dynamic_cast<const MatrixLiteral*>(&node)) {
        return {VarType::MATRIX, mat->rows(), mat->cols(), false, 0};
    }

    throw SemanticError("unknown expression type", 0, 1);
}
