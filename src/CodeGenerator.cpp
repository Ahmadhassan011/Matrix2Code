#include "CodeGenerator.h"

#include <cctype>
#include <unordered_map>

CodeGenerator::CodeGenerator(const IRProgram& program, const SymbolTable& symTable)
    : m_program(program), m_symTable(symTable) {}

void CodeGenerator::emitLine(std::string line) const {
    m_output += line + "\n";
}

bool CodeGenerator::isTemp(const std::string& name) const {
    return !name.empty() && name[0] == 't' && name.size() > 1 &&
           std::isdigit(name[1]);
}

struct VarInfo {
    bool isMatrix = false;
    int rows = 0;
    int cols = 0;
};

static std::unordered_map<std::string, VarInfo> buildVarMap(
        const IRProgram& program, const SymbolTable& symTable) {
    std::unordered_map<std::string, VarInfo> vars;

    for (const auto& instr : program.instructions()) {
        if (instr.op == IROp::SCALAR_DECL) {
            vars[instr.dst] = {false, 0, 0};
        } else if (instr.op == IROp::MATRIX_DECL) {
            vars[instr.dst] = {true, instr.dimM, instr.dimN};
        }
    }

    for (const auto& [name, info] : vars) {
        if (symTable.isDeclared(name)) {
            auto& sym = const_cast<SymbolTable&>(symTable).lookup(name);
            vars[name] = {sym.type == VarType::MATRIX, sym.matrixRows, sym.matrixCols};
        }
    }

    return vars;
}

static std::string varAccess(const std::string& name, const VarInfo& info,
                              const std::string& i, const std::string& j) {
    if (info.isMatrix) {
        return name + "[" + i + "][" + j + "]";
    }
    return name;
}

std::string CodeGenerator::generate() const {
    m_output.clear();

    auto varMap = buildVarMap(m_program, m_symTable);

    emitLine("#include <stdio.h>");
    emitLine("");
    emitLine("int main() {");

    for (const auto& instr : m_program.instructions()) {
        if (instr.op == IROp::SCALAR_DECL) {
            emitLine("    int " + instr.dst + ";");
        } else if (instr.op == IROp::MATRIX_DECL) {
            std::string decl = "    int " + instr.dst +
                               "[" + std::to_string(instr.dimM) + "]" +
                               "[" + std::to_string(instr.dimN) + "]";
            if (!instr.literalValues.empty()) {
                decl += " = {";
                for (size_t r = 0; r < instr.literalValues.size(); r++) {
                    if (r > 0) decl += ",";
                    decl += "{";
                    for (size_t c = 0; c < instr.literalValues[r].size(); c++) {
                        if (c > 0) decl += ",";
                        decl += std::to_string(instr.literalValues[r][c]);
                    }
                    decl += "}";
                }
                decl += "}";
            }
            decl += ";";
            emitLine(decl);
        }
    }

    for (const auto& instr : m_program.instructions()) {
        switch (instr.op) {
            case IROp::SCALAR_DECL:
            case IROp::MATRIX_DECL:
                break;

            case IROp::COPY: {
                auto dstInfo = varMap[instr.dst];
                auto srcInfo = varMap[instr.src1];
                if (dstInfo.isMatrix) {
                    emitLine("    for(int i=0; i<" + std::to_string(dstInfo.rows) + "; i++)");
                    emitLine("        for(int j=0; j<" + std::to_string(dstInfo.cols) + "; j++)");
                    emitLine("            " + instr.dst + "[i][j] = " +
                             varAccess(instr.src1, srcInfo, "i", "j") + ";");
                } else {
                    emitLine("    " + instr.dst + " = " + instr.src1 + ";");
                }
                break;
            }

            case IROp::ADD: {
                auto dstInfo = varMap[instr.dst];
                if (dstInfo.isMatrix) {
                    auto s1 = varMap[instr.src1];
                    auto s2 = varMap[instr.src2];
                    emitLine("    for(int i=0; i<" + std::to_string(dstInfo.rows) + "; i++)");
                    emitLine("        for(int j=0; j<" + std::to_string(dstInfo.cols) + "; j++)");
                    emitLine("            " + instr.dst + "[i][j] = " +
                             varAccess(instr.src1, s1, "i", "j") + " + " +
                             varAccess(instr.src2, s2, "i", "j") + ";");
                } else {
                    emitLine("    " + instr.dst + " = " + instr.src1 + " + " + instr.src2 + ";");
                }
                break;
            }

            case IROp::SUB: {
                auto dstInfo = varMap[instr.dst];
                if (dstInfo.isMatrix) {
                    auto s1 = varMap[instr.src1];
                    auto s2 = varMap[instr.src2];
                    emitLine("    for(int i=0; i<" + std::to_string(dstInfo.rows) + "; i++)");
                    emitLine("        for(int j=0; j<" + std::to_string(dstInfo.cols) + "; j++)");
                    emitLine("            " + instr.dst + "[i][j] = " +
                             varAccess(instr.src1, s1, "i", "j") + " - " +
                             varAccess(instr.src2, s2, "i", "j") + ";");
                } else {
                    emitLine("    " + instr.dst + " = " + instr.src1 + " - " + instr.src2 + ";");
                }
                break;
            }

            case IROp::MUL: {
                auto dstInfo = varMap[instr.dst];
                auto s1 = varMap[instr.src1];
                auto s2 = varMap[instr.src2];

                if (dstInfo.isMatrix && s1.isMatrix && s2.isMatrix) {
                    int kDim = s1.cols;
                    emitLine("    for(int i=0; i<" + std::to_string(dstInfo.rows) + "; i++)");
                    emitLine("        for(int j=0; j<" + std::to_string(dstInfo.cols) + "; j++) {");
                    emitLine("            " + instr.dst + "[i][j] = 0;");
                    emitLine("            for(int k=0; k<" + std::to_string(kDim) + "; k++)");
                    emitLine("                " + instr.dst + "[i][j] += " +
                             varAccess(instr.src1, s1, "i", "k") + " * " +
                             varAccess(instr.src2, s2, "k", "j") + ";");
                    emitLine("        }");
                } else if (dstInfo.isMatrix) {
                    emitLine("    for(int i=0; i<" + std::to_string(dstInfo.rows) + "; i++)");
                    emitLine("        for(int j=0; j<" + std::to_string(dstInfo.cols) + "; j++)");
                    emitLine("            " + instr.dst + "[i][j] = " +
                             varAccess(instr.src1, s1, "i", "j") + " * " +
                             varAccess(instr.src2, s2, "i", "j") + ";");
                } else {
                    emitLine("    " + instr.dst + " = " + instr.src1 + " * " + instr.src2 + ";");
                }
                break;
            }

            case IROp::PRINT: {
                auto info = varMap[instr.src1];
                if (info.isMatrix) {
                    emitLine("    for(int i=0; i<" + std::to_string(info.rows) + "; i++) {");
                    emitLine("        for(int j=0; j<" + std::to_string(info.cols) + "; j++)");
                    emitLine("            printf(\"%d \", " + instr.src1 + "[i][j]);");
                    emitLine("        printf(\"\\n\");");
                    emitLine("    }");
                } else {
                    emitLine("    printf(\"%d\\n\", " + instr.src1 + ");");
                }
                break;
            }
        }
    }

    emitLine("    return 0;");
    emitLine("}");

    return m_output;
}
