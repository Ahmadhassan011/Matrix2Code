#include "Lexer.h"

const std::unordered_map<std::string, TokenType> Lexer::s_keywords = {
    {"matrix", TokenType::MATRIX},
    {"print", TokenType::PRINT},
};

Lexer::Lexer(std::string source)
    : m_source(std::move(source))
    , m_pos(0)
    , m_line(1)
    , m_column(1)
    , m_hasPeeked(false) {}

bool Lexer::isAtEnd() const {
    return m_pos >= m_source.size();
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    char ch = m_source[m_pos++];
    if (ch == '\n') {
        m_line++;
        m_column = 1;
    } else {
        m_column++;
    }
    return ch;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return m_source[m_pos];
}

char Lexer::peekNext() const {
    if (m_pos + 1 >= m_source.size()) return '\0';
    return m_source[m_pos + 1];
}

Token Lexer::makeToken(TokenType type, std::string lexeme) {
    return {type, std::move(lexeme), m_line, m_column - static_cast<int>(lexeme.size())};
}

Token Lexer::makeToken(TokenType type, char ch) {
    return makeToken(type, std::string(1, ch));
}

void Lexer::skipWhitespaceExceptNewline() {
    while (!isAtEnd()) {
        char ch = peek();
        if (ch == ' ' || ch == '\t' || ch == '\r') {
            advance();
        } else {
            break;
        }
    }
}

void Lexer::skipLineComment() {
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

Token Lexer::number() {
    int startCol = m_column;
    std::string lexeme;
    while (!isAtEnd() && std::isdigit(peek())) {
        lexeme += advance();
    }
    return {TokenType::NUMBER, lexeme, m_line, startCol};
}

Token Lexer::identifierOrKeyword() {
    int startCol = m_column;
    std::string lexeme;
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        lexeme += advance();
    }
    auto it = s_keywords.find(lexeme);
    TokenType type = (it != s_keywords.end()) ? it->second : TokenType::IDENTIFIER;
    return {type, lexeme, m_line, startCol};
}

Token Lexer::newline() {
    int startCol = m_column;
    advance();
    return {TokenType::NEWLINE, "\\n", m_line - 1, startCol};
}

Token Lexer::nextToken() {
    if (m_hasPeeked) {
        m_hasPeeked = false;
        return m_peeked;
    }

    skipWhitespaceExceptNewline();

    if (isAtEnd()) {
        return makeToken(TokenType::END_OF_FILE, "");
    }

    char ch = peek();

    if (ch == '#') {
        skipLineComment();
        skipWhitespaceExceptNewline();
        if (isAtEnd()) return makeToken(TokenType::END_OF_FILE, "");
        ch = peek();
    }

    if (ch == '\n') {
        return newline();
    }

    if (ch == '+') { advance(); return makeToken(TokenType::PLUS, '+'); }
    if (ch == '-') { advance(); return makeToken(TokenType::MINUS, '-'); }
    if (ch == '*') { advance(); return makeToken(TokenType::STAR, '*'); }
    if (ch == '=') { advance(); return makeToken(TokenType::ASSIGN, '='); }
    if (ch == '[') { advance(); return makeToken(TokenType::LBRACKET, '['); }
    if (ch == ']') { advance(); return makeToken(TokenType::RBRACKET, ']'); }
    if (ch == '(') { advance(); return makeToken(TokenType::LPAREN, '('); }
    if (ch == ')') { advance(); return makeToken(TokenType::RPAREN, ')'); }
    if (ch == ',') { advance(); return makeToken(TokenType::COMMA, ','); }

    if (std::isdigit(ch)) {
        return number();
    }

    if (std::isalpha(ch) || ch == '_') {
        return identifierOrKeyword();
    }

    char bad = advance();
    throw LexicalError("unexpected character '" + std::string(1, bad) + "'", m_line, m_column - 1);
}

Token Lexer::peekToken() {
    if (!m_hasPeeked) {
        m_peeked = nextToken();
        m_hasPeeked = true;
    }
    return m_peeked;
}

bool Lexer::hasMore() const {
    if (m_hasPeeked) return true;
    size_t pos = m_pos;
    while (pos < m_source.size()) {
        char ch = m_source[pos];
        if (ch == ' ' || ch == '\t' || ch == '\r') { pos++; continue; }
        if (ch == '#') {
            pos++;
            while (pos < m_source.size() && m_source[pos] != '\n') pos++;
            continue;
        }
        return true;
    }
    return false;
}

std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;
    while (hasMore()) {
        tokens.push_back(nextToken());
    }
    tokens.push_back(makeToken(TokenType::END_OF_FILE, ""));
    return tokens;
}
