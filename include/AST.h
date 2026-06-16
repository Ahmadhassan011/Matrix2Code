#pragma once

#include <memory>
#include <string>
#include <vector>

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

class NumberLiteral : public ASTNode {
public:
    explicit NumberLiteral(int value) : m_value(value) {}
    int value() const { return m_value; }
    void print(int indent = 0) const override;

private:
    int m_value;
};

class Identifier : public ASTNode {
public:
    explicit Identifier(std::string name) : m_name(std::move(name)) {}
    const std::string& name() const { return m_name; }
    void print(int indent = 0) const override;

private:
    std::string m_name;
};

class BinaryOp : public ASTNode {
public:
    enum Op { ADD, SUB, MUL, DIV };

    BinaryOp(Op op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right, int line)
        : m_op(op), m_left(std::move(left)), m_right(std::move(right)), m_line(line) {}

    Op op() const { return m_op; }
    int line() const { return m_line; }
    const ASTNode* left() const { return m_left.get(); }
    const ASTNode* right() const { return m_right.get(); }

    void print(int indent = 0) const override;

private:
    Op m_op;
    std::unique_ptr<ASTNode> m_left;
    std::unique_ptr<ASTNode> m_right;
    int m_line;
};

class MatrixLiteral : public ASTNode {
public:
    MatrixLiteral(std::vector<std::vector<int>> values, int rows, int cols)
        : m_values(std::move(values)), m_rows(rows), m_cols(cols) {}

    const std::vector<std::vector<int>>& values() const { return m_values; }
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    void print(int indent = 0) const override;

private:
    std::vector<std::vector<int>> m_values;
    int m_rows;
    int m_cols;
};

class Statement : public ASTNode {
public:
    virtual int line() const = 0;
};

class MatrixDecl : public Statement {
public:
    MatrixDecl(std::string name, std::unique_ptr<MatrixLiteral> literal, int line)
        : m_name(std::move(name)), m_literal(std::move(literal)), m_line(line) {}

    const std::string& name() const { return m_name; }
    MatrixLiteral* literal() const { return m_literal.get(); }
    int line() const override { return m_line; }

    void print(int indent = 0) const override;

private:
    std::string m_name;
    std::unique_ptr<MatrixLiteral> m_literal;
    int m_line;
};

class AssignStmt : public Statement {
public:
    AssignStmt(std::string name, std::unique_ptr<ASTNode> expr, int line)
        : m_name(std::move(name)), m_expr(std::move(expr)), m_line(line) {}

    const std::string& name() const { return m_name; }
    const ASTNode* expr() const { return m_expr.get(); }
    int line() const override { return m_line; }

    void print(int indent = 0) const override;

private:
    std::string m_name;
    std::unique_ptr<ASTNode> m_expr;
    int m_line;
};

class TransposeExpr : public ASTNode {
public:
    TransposeExpr(std::unique_ptr<ASTNode> expr, int line)
        : m_expr(std::move(expr)), m_line(line) {}

    const ASTNode* expr() const { return m_expr.get(); }
    int line() const { return m_line; }
    void print(int indent = 0) const override;

private:
    std::unique_ptr<ASTNode> m_expr;
    int m_line;
};

class DetExpr : public ASTNode {
public:
    DetExpr(std::unique_ptr<ASTNode> expr, int line)
        : m_expr(std::move(expr)), m_line(line) {}

    const ASTNode* expr() const { return m_expr.get(); }
    int line() const { return m_line; }
    void print(int indent = 0) const override;

private:
    std::unique_ptr<ASTNode> m_expr;
    int m_line;
};

class PrintStmt : public Statement {
public:
    PrintStmt(std::string name, int line)
        : m_name(std::move(name)), m_line(line) {}

    const std::string& name() const { return m_name; }
    int line() const override { return m_line; }

    void print(int indent = 0) const override;

private:
    std::string m_name;
    int m_line;
};

class Program : public ASTNode {
public:
    explicit Program(std::vector<std::unique_ptr<Statement>> statements)
        : m_statements(std::move(statements)) {}

    const std::vector<std::unique_ptr<Statement>>& statements() const { return m_statements; }

    void print(int indent = 0) const override;

private:
    std::vector<std::unique_ptr<Statement>> m_statements;
};
