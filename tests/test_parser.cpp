#include "doctest.h"
#include "Parser.h"
#include "Lexer.h"

TEST_CASE("Parser - empty program") {
    Lexer lexer("");
    Parser parser(lexer);
    auto program = parser.parse();
    CHECK(program->statements().empty());
}

TEST_CASE("Parser - scalar assignment") {
    Lexer lexer("x = 42");
    Parser parser(lexer);
    auto program = parser.parse();
    REQUIRE(program->statements().size() == 1);
    auto* stmt = dynamic_cast<AssignStmt*>(program->statements()[0].get());
    REQUIRE(stmt != nullptr);
    CHECK(stmt->name() == "x");
}

TEST_CASE("Parser - matrix declaration") {
    Lexer lexer("matrix A = [[1,2],[3,4]]");
    Parser parser(lexer);
    auto program = parser.parse();
    REQUIRE(program->statements().size() == 1);
    auto* stmt = dynamic_cast<MatrixDecl*>(program->statements()[0].get());
    REQUIRE(stmt != nullptr);
    CHECK(stmt->name() == "A");
    CHECK(stmt->literal()->rows() == 2);
    CHECK(stmt->literal()->cols() == 2);
}

TEST_CASE("Parser - print statement") {
    Lexer lexer("print x");
    Parser parser(lexer);
    auto program = parser.parse();
    REQUIRE(program->statements().size() == 1);
    auto* stmt = dynamic_cast<PrintStmt*>(program->statements()[0].get());
    REQUIRE(stmt != nullptr);
    CHECK(stmt->name() == "x");
}

TEST_CASE("Parser - operator precedence: a + b * c") {
    Lexer lexer("x = 1 + 2 * 3");
    Parser parser(lexer);
    auto program = parser.parse();
    auto* stmt = dynamic_cast<AssignStmt*>(program->statements()[0].get());
    REQUIRE(stmt != nullptr);
    auto* binop = dynamic_cast<const BinaryOp*>(stmt->expr());
    REQUIRE(binop != nullptr);
    CHECK(binop->op() == BinaryOp::ADD);
    auto* right = dynamic_cast<const BinaryOp*>(binop->right());
    REQUIRE(right != nullptr);
    CHECK(right->op() == BinaryOp::MUL);
}

TEST_CASE("Parser - parenthesized expression") {
    Lexer lexer("x = (1 + 2) * 3");
    Parser parser(lexer);
    auto program = parser.parse();
    auto* stmt = dynamic_cast<AssignStmt*>(program->statements()[0].get());
    REQUIRE(stmt != nullptr);
    auto* binop = dynamic_cast<const BinaryOp*>(stmt->expr());
    REQUIRE(binop != nullptr);
    CHECK(binop->op() == BinaryOp::MUL);
    auto* left = dynamic_cast<const BinaryOp*>(binop->left());
    REQUIRE(left != nullptr);
    CHECK(left->op() == BinaryOp::ADD);
}

TEST_CASE("Parser - matrix literal row length validation") {
    Lexer lexer("matrix A = [[1,2],[3]]");
    Parser parser(lexer);
    CHECK_THROWS_AS(parser.parse(), SemanticError);
}

TEST_CASE("Parser - division operator") {
    Lexer lexer("x = 10 / 3");
    Parser parser(lexer);
    auto program = parser.parse();
    auto* stmt = dynamic_cast<AssignStmt*>(program->statements()[0].get());
    REQUIRE(stmt != nullptr);
    auto* binop = dynamic_cast<const BinaryOp*>(stmt->expr());
    REQUIRE(binop != nullptr);
    CHECK(binop->op() == BinaryOp::DIV);
}

TEST_CASE("Parser - transpose expression") {
    Lexer lexer("matrix A = [[1,2],[3,4]]\nB = transpose(A)");
    Parser parser(lexer);
    auto program = parser.parse();
    REQUIRE(program->statements().size() == 2);
    auto* stmt = dynamic_cast<AssignStmt*>(program->statements()[1].get());
    REQUIRE(stmt != nullptr);
    auto* trans = dynamic_cast<const TransposeExpr*>(stmt->expr());
    REQUIRE(trans != nullptr);
}

TEST_CASE("Parser - det expression") {
    Lexer lexer("matrix A = [[1,2],[3,4]]\nd = det(A)");
    Parser parser(lexer);
    auto program = parser.parse();
    REQUIRE(program->statements().size() == 2);
    auto* stmt = dynamic_cast<AssignStmt*>(program->statements()[1].get());
    REQUIRE(stmt != nullptr);
    auto* det = dynamic_cast<const DetExpr*>(stmt->expr());
    REQUIRE(det != nullptr);
}

TEST_CASE("Parser - division and multiplication precedence") {
    Lexer lexer("x = 10 / 2 * 3");
    Parser parser(lexer);
    auto program = parser.parse();
    auto* stmt = dynamic_cast<AssignStmt*>(program->statements()[0].get());
    REQUIRE(stmt != nullptr);
    auto* binop = dynamic_cast<const BinaryOp*>(stmt->expr());
    REQUIRE(binop != nullptr);
    CHECK(binop->op() == BinaryOp::MUL);
    auto* left = dynamic_cast<const BinaryOp*>(binop->left());
    REQUIRE(left != nullptr);
    CHECK(left->op() == BinaryOp::DIV);
}
