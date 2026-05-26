#pragma once

#include <memory>
#include <vector>

#include "AST.h"
#include "Lexer.h"

class Parser {
public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<Program> parse();

private:
    using StmtPtr = std::unique_ptr<Statement>;
    using ExprPtr = std::unique_ptr<ASTNode>;

    StmtPtr parseStatement();
    StmtPtr parseMatrixDecl();
    StmtPtr parseAssignment();
    StmtPtr parsePrintStmt();
    ExprPtr parseExpression();
    ExprPtr parseTerm();
    ExprPtr parseFactor();
    std::unique_ptr<MatrixLiteral> parseMatrixLiteral();

    Token consume();
    Token expect(TokenType type);
    Token peek() const;
    Token previous() const;
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool isAtEnd() const;
    void consumeNewlines();

    Token advance();

    Lexer& m_lexer;
    Token m_current;
    Token m_previous;
    bool m_hasCurrent;
};
