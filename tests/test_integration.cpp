#include "doctest.h"
#include "CodeGenerator.h"
#include "IR.h"
#include "IRGenerator.h"
#include "Lexer.h"
#include "Optimizer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "SymbolTable.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>

static std::string compileAndRun(const std::string& source) {
    Lexer lexer(source);
    Parser parser(lexer);
    auto program = parser.parse();

    SymbolTable symTable;
    SemanticAnalyzer analyzer(symTable);
    analyzer.analyze(*program);

    IRProgram ir;
    IRGenerator irGen(ir, symTable);
    irGen.generate(*program);

    Optimizer optimizer;
    optimizer.optimize(ir);

    CodeGenerator codeGen(ir, symTable);
    std::string cCode = codeGen.generate();

    {
        std::ofstream outFile("test_output.c");
        outFile << cCode;
    }

    int compileResult = std::system("gcc -Wall -o test_output test_output.c 2>/dev/null");
    if (compileResult != 0) {
        return "COMPILE_ERROR";
    }

    std::string result;
    {
        std::array<char, 128> buffer;
        auto pipeDeleter = [](FILE* f) { if (f) pclose(f); };
        std::unique_ptr<FILE, decltype(pipeDeleter)> pipe(popen("./test_output 2>/dev/null", "r"), pipeDeleter);
        if (!pipe) return "RUNTIME_ERROR";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }

    std::system("rm -f test_output test_output.c");
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

TEST_CASE("Integration - matrix addition") {
    std::string src = "matrix A = [[1,2],[3,4]]\nmatrix B = [[5,6],[7,8]]\nC = A + B\nprint C";
    CHECK(compileAndRun(src) == "6 8 \n10 12 ");
}

TEST_CASE("Integration - matrix multiplication") {
    std::string src = "matrix A = [[1,2],[3,4]]\nmatrix B = [[5,6],[7,8]]\nC = A * B\nprint C";
    CHECK(compileAndRun(src) == "19 22 \n43 50 ");
}

TEST_CASE("Integration - scalar arithmetic") {
    std::string src = "x = 5 + 3 * 2\nprint x";
    CHECK(compileAndRun(src) == "11");
}

TEST_CASE("Integration - scalar-matrix scaling") {
    std::string src = "matrix A = [[1,2],[3,4]]\nB = 2 * A\nprint B";
    CHECK(compileAndRun(src) == "2 4 \n6 8 ");
}

TEST_CASE("Integration - matrix subtraction") {
    std::string src = "matrix A = [[5,6],[7,8]]\nmatrix B = [[1,2],[3,4]]\nC = A - B\nprint C";
    CHECK(compileAndRun(src) == "4 4 \n4 4 ");
}

TEST_CASE("Integration - chained expressions") {
    std::string src = "x = 2 * 3 + 4\nprint x";
    CHECK(compileAndRun(src) == "10");
}

TEST_CASE("Integration - multiple prints") {
    std::string src = "x = 10\ny = 20\nprint x\nprint y";
    CHECK(compileAndRun(src) == "10\n20");
}

TEST_CASE("Integration - implicit scalar decl") {
    std::string src = "x = 42\nprint x";
    CHECK(compileAndRun(src) == "42");
}

TEST_CASE("Integration - matrix right scaling") {
    std::string src = "matrix A = [[1,2],[3,4]]\nC = A * 3\nprint C";
    CHECK(compileAndRun(src) == "3 6 \n9 12 ");
}
