#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ErrorHandler.h"

enum class TokenType {
    NUMBER,
    IDENTIFIER,
    PLUS,
    MINUS,
    STAR,
    ASSIGN,
    LBRACKET,
    RBRACKET,
    LPAREN,
    RPAREN,
    COMMA,
    MATRIX,
    PRINT,
    NEWLINE,
    END_OF_FILE,
    INVALID
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    Token nextToken();
    Token peekToken();
    std::vector<Token> tokenizeAll();

    bool hasMore() const;
    int line() const { return m_line; }
    int column() const { return m_column; }

private:
    char advance();
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;
    void skipLineComment();
    void skipWhitespaceExceptNewline();
    Token makeToken(TokenType type, std::string lexeme);
    Token makeToken(TokenType type, char ch);

    Token number();
    Token identifierOrKeyword();
    Token newline();

    std::string m_source;
    size_t m_pos;
    int m_line;
    int m_column;
    Token m_peeked;
    bool m_hasPeeked;

    static const std::unordered_map<std::string, TokenType> s_keywords;
};
