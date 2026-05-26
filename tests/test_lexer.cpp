#include "doctest.h"
#include "Lexer.h"

TEST_CASE("Lexer - basic tokens") {
    Lexer lexer("+ - * = , [ ] ( )");
    auto tokens = lexer.tokenizeAll();

    CHECK(tokens.size() == 10);
    CHECK(tokens[0].type == TokenType::PLUS);
    CHECK(tokens[1].type == TokenType::MINUS);
    CHECK(tokens[2].type == TokenType::STAR);
    CHECK(tokens[3].type == TokenType::ASSIGN);
    CHECK(tokens[4].type == TokenType::COMMA);
    CHECK(tokens[5].type == TokenType::LBRACKET);
    CHECK(tokens[6].type == TokenType::RBRACKET);
    CHECK(tokens[7].type == TokenType::LPAREN);
    CHECK(tokens[8].type == TokenType::RPAREN);
    CHECK(tokens[9].type == TokenType::END_OF_FILE);
}

TEST_CASE("Lexer - numbers and identifiers") {
    Lexer lexer("42 abc");
    auto tokens = lexer.tokenizeAll();

    CHECK(tokens[0].type == TokenType::NUMBER);
    CHECK(tokens[0].lexeme == "42");
    CHECK(tokens[1].type == TokenType::IDENTIFIER);
    CHECK(tokens[1].lexeme == "abc");
}

TEST_CASE("Lexer - keywords") {
    Lexer lexer("matrix print");
    auto tokens = lexer.tokenizeAll();

    CHECK(tokens[0].type == TokenType::MATRIX);
    CHECK(tokens[0].lexeme == "matrix");
    CHECK(tokens[1].type == TokenType::PRINT);
    CHECK(tokens[1].lexeme == "print");
}

TEST_CASE("Lexer - newlines separate statements") {
    Lexer lexer("x = 1\ny = 2");
    auto tokens = lexer.tokenizeAll();

    CHECK(tokens[3].type == TokenType::NEWLINE);
    CHECK(tokens[3].line == 1);
}

TEST_CASE("Lexer - comments are skipped") {
    Lexer lexer("x = 1  # this is a comment\ny = 2");
    auto tokens = lexer.tokenizeAll();

    for (const auto& t : tokens) {
        CHECK(t.type != TokenType::INVALID);
    }
    CHECK(tokens[2].type == TokenType::NUMBER);
    CHECK(tokens[2].lexeme == "1");
}

TEST_CASE("Lexer - unrecognized character") {
    Lexer lexer("x = @ 1");
    try {
        lexer.tokenizeAll();
        FAIL("expected exception");
    } catch (const LexicalError& e) {
        CHECK(std::string(e.what()).find("unexpected character") != std::string::npos);
    }
}

TEST_CASE("Lexer - empty input") {
    Lexer lexer("");
    auto tokens = lexer.tokenizeAll();
    CHECK(tokens.size() == 1);
    CHECK(tokens[0].type == TokenType::END_OF_FILE);
}
