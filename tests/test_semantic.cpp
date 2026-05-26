#include "doctest.h"
#include "Lexer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "SymbolTable.h"

static std::unique_ptr<Program> parse(const std::string& src) {
    auto lexer = std::make_unique<Lexer>(src);
    auto parser = std::make_unique<Parser>(*lexer);
    return parser->parse();
}

TEST_CASE("SemanticAnalyzer - valid program") {
    auto program = parse("x = 5\ny = x + 3\nprint y");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    CHECK_NOTHROW(analyzer.analyze(*program));
    CHECK(symTable.size() == 2);
}

TEST_CASE("SemanticAnalyzer - undeclared variable") {
    auto program = parse("x = y + 1");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    CHECK_THROWS_AS(analyzer.analyze(*program), SemanticError);
}

TEST_CASE("SemanticAnalyzer - redeclared matrix") {
    auto program = parse("matrix A = [[1,2],[3,4]]\nmatrix A = [[5,6],[7,8]]");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    CHECK_THROWS_AS(analyzer.analyze(*program), SemanticError);
}

TEST_CASE("SemanticAnalyzer - type mismatch scalar + matrix") {
    auto program = parse("matrix A = [[1,2],[3,4]]\nx = A + 5");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    CHECK_THROWS_AS(analyzer.analyze(*program), SemanticError);
}

TEST_CASE("SemanticAnalyzer - dimension mismatch addition") {
    auto program = parse("matrix A = [[1,2],[3,4]]\nmatrix B = [[1,2,3],[4,5,6]]\nC = A + B");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    CHECK_THROWS_AS(analyzer.analyze(*program), SemanticError);
}

TEST_CASE("SemanticAnalyzer - dimension mismatch multiplication") {
    auto program = parse("matrix A = [[1,2],[3,4]]\nmatrix B = [[1,2],[3,4]]\nC = A * B");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    CHECK_NOTHROW(analyzer.analyze(*program));
}

TEST_CASE("SemanticAnalyzer - scalar constant folding") {
    auto program = parse("x = 5 + 3 * 2");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    analyzer.analyze(*program);
    auto info = symTable.lookup("x");
    CHECK(info.type == VarType::SCALAR);
}

TEST_CASE("SemanticAnalyzer - matrix scaling") {
    auto program = parse("matrix A = [[1,2],[3,4]]\nB = 2 * A\nprint B");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    CHECK_NOTHROW(analyzer.analyze(*program));
    auto info = symTable.lookup("B");
    CHECK(info.type == VarType::MATRIX);
    CHECK(info.matrixRows == 2);
    CHECK(info.matrixCols == 2);
}

TEST_CASE("SemanticAnalyzer - invalid matrix literal too small") {
    auto program = parse("matrix A = [[1]]");
    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    CHECK_NOTHROW(analyzer.analyze(*program));
}
