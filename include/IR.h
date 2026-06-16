#pragma once

#include <string>
#include <vector>

enum class IROp {
    SCALAR_DECL,
    MATRIX_DECL,
    COPY,
    ADD,
    SUB,
    MUL,
    DIV,
    TRANSPOSE,
    DET,
    PRINT
};

struct IRInstr {
    IROp op;
    std::string dst;
    std::string src1;
    std::string src2;
    int dimM = 0;
    int dimN = 0;
    std::vector<std::vector<int>> literalValues;
};

class IRProgram {
public:
    void emit(IROp op, std::string dst,
              std::string src1 = "",
              std::string src2 = "",
              int dimM = 0, int dimN = 0,
              std::vector<std::vector<int>> literalValues = {});

    const std::vector<IRInstr>& instructions() const { return m_instrs; }
    std::vector<IRInstr>& instructions() { return m_instrs; }
    size_t size() const { return m_instrs.size(); }

    void dump() const;

private:
    std::vector<IRInstr> m_instrs;
    int m_tempCounter = 0;

    friend class IRGenerator;
    std::string newTemp();
};
