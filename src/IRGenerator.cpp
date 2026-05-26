#include "IRGenerator.h"
#include "ErrorHandler.h"

IRGenerator::IRGenerator(IRProgram& program, SymbolTable& symTable)
    : m_program(program), m_symTable(symTable) {}

void IRGenerator::generate(const Program& ast) {
    for (const auto& stmt : ast.statements()) {
        generateStmt(*stmt);
    }
}

void IRGenerator::generateStmt(const Statement& stmt) {
    if (auto* md = dynamic_cast<const MatrixDecl*>(&stmt)) {
        generateMatrixDecl(*md);
    } else if (auto* as = dynamic_cast<const AssignStmt*>(&stmt)) {
        generateAssign(*as);
    } else if (auto* ps = dynamic_cast<const PrintStmt*>(&stmt)) {
        generatePrint(*ps);
    }
}

void IRGenerator::generateMatrixDecl(const MatrixDecl& stmt) {
    m_program.emit(IROp::MATRIX_DECL, stmt.name(),
                   "", "", stmt.literal()->rows(), stmt.literal()->cols(),
                   stmt.literal()->values());
}

void IRGenerator::generateAssign(const AssignStmt& stmt) {
    SymbolInfo& sym = m_symTable.lookup(stmt.name());
    ExprResult result = translateExpr(*stmt.expr(), stmt.name());

    if (sym.type == VarType::SCALAR && result.ref != stmt.name()) {
        m_program.emit(IROp::SCALAR_DECL, stmt.name());
    }

    if (result.ref != stmt.name()) {
        if (sym.type == VarType::MATRIX) {
            m_program.emit(IROp::COPY, stmt.name(), result.ref,
                           "", sym.matrixRows, sym.matrixCols);
        } else {
            m_program.emit(IROp::COPY, stmt.name(), result.ref);
        }
    }
}

void IRGenerator::generatePrint(const PrintStmt& stmt) {
    m_program.emit(IROp::PRINT, "", stmt.name());
}

IRGenerator::ExprResult IRGenerator::translateExpr(
        const ASTNode& node, const std::string& dstHint) {

    if (auto* num = dynamic_cast<const NumberLiteral*>(&node)) {
        return {std::to_string(num->value()), VarType::SCALAR, 0, 0};
    }

    if (auto* ident = dynamic_cast<const Identifier*>(&node)) {
        SymbolInfo& sym = m_symTable.lookup(ident->name());
        return {ident->name(), sym.type, sym.matrixRows, sym.matrixCols};
    }

    if (auto* binop = dynamic_cast<const BinaryOp*>(&node)) {
        ExprResult left = translateExpr(*binop->left());
        ExprResult right = translateExpr(*binop->right());

        VarType resultType;
        int resultRows = 0, resultCols = 0;

        if (left.type == VarType::MATRIX || right.type == VarType::MATRIX) {
            resultType = VarType::MATRIX;
            resultRows = (left.type == VarType::MATRIX) ? left.rows : right.rows;
            resultCols = (left.type == VarType::MATRIX) ? left.cols : right.cols;
            if (binop->op() == BinaryOp::MUL && left.type == VarType::MATRIX && right.type == VarType::MATRIX) {
                resultRows = left.rows;
                resultCols = right.cols;
            }
        } else {
            resultType = VarType::SCALAR;
        }

        std::string dst = dstHint.empty() ? m_program.newTemp() : dstHint;

        if (resultType == VarType::MATRIX) {
            m_program.emit(IROp::MATRIX_DECL, dst, "", "", resultRows, resultCols);
        } else {
            m_program.emit(IROp::SCALAR_DECL, dst);
        }

        IROp op;
        switch (binop->op()) {
            case BinaryOp::ADD: op = IROp::ADD; break;
            case BinaryOp::SUB: op = IROp::SUB; break;
            case BinaryOp::MUL: op = IROp::MUL; break;
        }

        m_program.emit(op, dst, left.ref, right.ref, resultRows, resultCols);
        return {dst, resultType, resultRows, resultCols};
    }

    if (auto* mat = dynamic_cast<const MatrixLiteral*>(&node)) {
        std::string dst = dstHint.empty() ? m_program.newTemp() : dstHint;
        m_program.emit(IROp::MATRIX_DECL, dst, "", "", mat->rows(), mat->cols());
        return {dst, VarType::MATRIX, mat->rows(), mat->cols()};
    }

    throw SemanticError("unknown expression in IR generation", 0, 1);
}
