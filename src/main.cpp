#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "CodeGenerator.h"
#include "ErrorHandler.h"
#include "IR.h"
#include "IRGenerator.h"
#include "Lexer.h"
#include "Optimizer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "SymbolTable.h"

#include <fstream>

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file '" << path << "'\n";
        std::exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static void printUsage() {
    std::cout << "Usage: compiler <source.matrix> [-o <output.c>]\n";
    std::cout << "       [--no-run] [--dump-ast] [--dump-ir] [--help]\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string sourcePath;
    std::string outputPath = "output.c";
    bool noRun = false;
    bool dumpAST = false;
    bool dumpIR = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "-o" && i + 1 < argc) {
            outputPath = argv[++i];
        } else if (arg == "--no-run") {
            noRun = true;
        } else if (arg == "--dump-ast") {
            dumpAST = true;
        } else if (arg == "--dump-ir") {
            dumpIR = true;
        } else if (arg[0] != '-') {
            sourcePath = arg;
        }
    }

    if (sourcePath.empty()) {
        std::cerr << "Error: no input file specified\n";
        printUsage();
        return 1;
    }

    try {
        std::string source = readFile(sourcePath);

        Lexer lexer(source);
        Parser parser(lexer);
        auto program = parser.parse();

        SymbolTable symTable;
        SemanticAnalyzer analyzer(symTable);
        analyzer.analyze(*program);

        if (dumpAST) {
            program->print();
        }

        std::cout << "Semantic analysis passed. " << symTable.size() << " variables.\n";

        IRProgram ir;
        IRGenerator irGen(ir, symTable);
        irGen.generate(*program);

        if (dumpIR) {
            std::cout << "--- IR (before optimization) ---\n";
            ir.dump();
        }

        Optimizer optimizer;
        optimizer.optimize(ir);

        if (dumpIR) {
            std::cout << "--- IR (after optimization) ---\n";
            ir.dump();
            std::cout << "---\n";
        }

        CodeGenerator codeGen(ir, symTable);
        std::string cCode = codeGen.generate();

        {
            std::ofstream outFile(outputPath);
            if (!outFile.is_open()) {
                std::cerr << "Error: could not write to '" << outputPath << "'\n";
                return 1;
            }
            outFile << cCode;
        }

        std::cout << "Generated " << outputPath << " (" << cCode.size() << " bytes)\n";

        if (!noRun) {
            std::string compileCmd = "gcc -Wall -o output " + outputPath + " 2>&1";
            int compileResult = std::system(compileCmd.c_str());
            if (compileResult != 0) {
                std::cerr << "Error: GCC compilation failed (exit code " << compileResult << ")\n";
                return 1;
            }
            int runResult = std::system("./output");
            if (runResult != 0) {
                std::cerr << "Error: program exited with code " << runResult << "\n";
                return 2;
            }
        }

    } catch (const CompilerError& e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Internal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
