#include "Parser.h"

Parser::Parser(Lexer& lexer)
    : m_lexer(lexer)
    , m_current({TokenType::END_OF_FILE, "", 0, 0})
    , m_previous({TokenType::END_OF_FILE, "", 0, 0})
    , m_hasCurrent(false) {}

Token Parser::advance() {
    m_previous = m_current;
    m_current = m_lexer.nextToken();
    return m_previous;
}

Token Parser::peek() const {
    return m_current;
}

Token Parser::previous() const {
    return m_previous;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return m_current.type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::isAtEnd() const {
    return m_current.type == TokenType::END_OF_FILE;
}

Token Parser::consume() {
    return advance();
}

Token Parser::expect(TokenType type) {
    if (check(type)) {
        return advance();
    }
    throw SyntaxError(
        "expected " + std::to_string(static_cast<int>(type)) +
        " but got '" + m_current.lexeme + "'",
        m_current.line, m_current.column
    );
}

void Parser::consumeNewlines() {
    while (match(TokenType::NEWLINE)) {}
}

std::unique_ptr<Program> Parser::parse() {
    advance();

    std::vector<std::unique_ptr<Statement>> statements;

    consumeNewlines();

    while (!isAtEnd()) {
        statements.push_back(parseStatement());

        if (isAtEnd()) break;

        if (!match(TokenType::NEWLINE)) {
            throw SyntaxError(
                "expected newline after statement, got '" + m_current.lexeme + "'",
                m_current.line, m_current.column
            );
        }

        consumeNewlines();
    }

    return std::make_unique<Program>(std::move(statements));
}

Parser::StmtPtr Parser::parseStatement() {
    if (match(TokenType::MATRIX)) {
        return parseMatrixDecl();
    }
    if (match(TokenType::PRINT)) {
        return parsePrintStmt();
    }
    if (check(TokenType::IDENTIFIER)) {
        return parseAssignment();
    }
    throw SyntaxError(
        "unexpected token '" + m_current.lexeme + "'",
        m_current.line, m_current.column
    );
}

Parser::StmtPtr Parser::parseMatrixDecl() {
    Token name = expect(TokenType::IDENTIFIER);
    expect(TokenType::ASSIGN);
    auto literal = parseMatrixLiteral();
    return std::make_unique<MatrixDecl>(name.lexeme, std::move(literal), name.line);
}

Parser::StmtPtr Parser::parseAssignment() {
    Token name = consume();
    expect(TokenType::ASSIGN);
    auto expr = parseExpression();
    return std::make_unique<AssignStmt>(name.lexeme, std::move(expr), name.line);
}

Parser::StmtPtr Parser::parsePrintStmt() {
    Token name = expect(TokenType::IDENTIFIER);
    return std::make_unique<PrintStmt>(name.lexeme, name.line);
}

std::unique_ptr<MatrixLiteral> Parser::parseMatrixLiteral() {
    expect(TokenType::LBRACKET);

    std::vector<std::vector<int>> rows;
    int rowNumber = 0;

    do {
        expect(TokenType::LBRACKET);
        std::vector<int> row;
        int firstColLine = m_current.line;

        do {
            bool negative = match(TokenType::MINUS);
            Token num = expect(TokenType::NUMBER);
            int val = std::stoi(num.lexeme);
            row.push_back(negative ? -val : val);
        } while (match(TokenType::COMMA));

        expect(TokenType::RBRACKET);

        if (!rows.empty() && row.size() != rows[0].size()) {
            throw SemanticError(
                "matrix literal has inconsistent row lengths (row 0: " +
                std::to_string(rows[0].size()) + " cols, row " +
                std::to_string(rows.size()) + ": " +
                std::to_string(row.size()) + " cols)",
                firstColLine, 1
            );
        }

        rows.push_back(std::move(row));
        rowNumber++;
    } while (match(TokenType::COMMA));

    expect(TokenType::RBRACKET);

    if (rows.empty() || rows[0].empty()) {
        throw SemanticError(
            "matrix literal must have at least 1 row and 1 column",
            m_previous.line, m_previous.column
        );
    }

    int numRows = static_cast<int>(rows.size());
    int numCols = static_cast<int>(rows[0].size());
    return std::make_unique<MatrixLiteral>(std::move(rows), numRows, numCols);
}

Parser::ExprPtr Parser::parseExpression() {
    auto expr = parseTerm();

    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        BinaryOp::Op op = (m_previous.type == TokenType::PLUS) ? BinaryOp::ADD : BinaryOp::SUB;
        int line = m_previous.line;
        auto right = parseTerm();
        expr = std::make_unique<BinaryOp>(op, std::move(expr), std::move(right), line);
    }

    return expr;
}

Parser::ExprPtr Parser::parseTerm() {
    auto expr = parseFactor();

    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        BinaryOp::Op op = (m_previous.type == TokenType::STAR) ? BinaryOp::MUL : BinaryOp::DIV;
        int line = m_previous.line;
        auto right = parseFactor();
        expr = std::make_unique<BinaryOp>(op, std::move(expr), std::move(right), line);
    }

    return expr;
}

Parser::ExprPtr Parser::parseFactor() {
    if (match(TokenType::NUMBER)) {
        return std::make_unique<NumberLiteral>(std::stoi(m_previous.lexeme));
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<Identifier>(m_previous.lexeme);
    }

    if (match(TokenType::TRANSPOSE)) {
        int line = m_previous.line;
        expect(TokenType::LPAREN);
        auto expr = parseExpression();
        expect(TokenType::RPAREN);
        return std::make_unique<TransposeExpr>(std::move(expr), line);
    }

    if (match(TokenType::DET)) {
        int line = m_previous.line;
        expect(TokenType::LPAREN);
        auto expr = parseExpression();
        expect(TokenType::RPAREN);
        return std::make_unique<DetExpr>(std::move(expr), line);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        expect(TokenType::RPAREN);
        return expr;
    }

    throw SyntaxError(
        "expected expression, got '" + m_current.lexeme + "'",
        m_current.line, m_current.column
    );
}
