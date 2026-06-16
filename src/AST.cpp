#include "AST.h"

#include <iostream>

static void printIndent(int n) { for (int i = 0; i < n; i++) std::cerr << "  "; }

void NumberLiteral::print(int indent) const {
    printIndent(indent);
    std::cerr << "Number: " << m_value << "\n";
}

void Identifier::print(int indent) const {
    printIndent(indent);
    std::cerr << "Identifier: " << m_name << "\n";
}

void BinaryOp::print(int indent) const {
    const char* opStr = "";
    switch (m_op) {
        case ADD: opStr = "+"; break;
        case SUB: opStr = "-"; break;
        case MUL: opStr = "*"; break;
        case DIV: opStr = "/"; break;
    }
    printIndent(indent);
    std::cerr << "BinaryOp: " << opStr << "\n";
    if (m_left) m_left->print(indent + 1);
    if (m_right) m_right->print(indent + 1);
}

void MatrixLiteral::print(int indent) const {
    printIndent(indent);
    std::cerr << "MatrixLiteral: " << m_rows << "x" << m_cols << "\n";
    for (int i = 0; i < m_rows; i++) {
        printIndent(indent + 1);
        std::cerr << "[";
        for (int j = 0; j < m_cols; j++) {
            if (j > 0) std::cerr << ", ";
            std::cerr << m_values[i][j];
        }
        std::cerr << "]\n";
    }
}

void MatrixDecl::print(int indent) const {
    printIndent(indent);
    std::cerr << "MatrixDecl: " << m_name << "\n";
    if (m_literal) m_literal->print(indent + 1);
}

void AssignStmt::print(int indent) const {
    printIndent(indent);
    std::cerr << "Assign: " << m_name << "\n";
    if (m_expr) m_expr->print(indent + 1);
}

void TransposeExpr::print(int indent) const {
    printIndent(indent);
    std::cerr << "Transpose\n";
    if (m_expr) m_expr->print(indent + 1);
}

void DetExpr::print(int indent) const {
    printIndent(indent);
    std::cerr << "Det\n";
    if (m_expr) m_expr->print(indent + 1);
}

void PrintStmt::print(int indent) const {
    printIndent(indent);
    std::cerr << "Print: " << m_name << "\n";
}

void Program::print(int indent) const {
    std::cerr << "Program\n";
    for (const auto& stmt : m_statements) {
        if (stmt) stmt->print(indent + 1);
    }
}
