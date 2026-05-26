#include "IR.h"

#include <iostream>

void IRProgram::emit(IROp op, std::string dst,
                     std::string src1, std::string src2,
                     int dimM, int dimN,
                     std::vector<std::vector<int>> literalValues) {
    m_instrs.push_back({op, std::move(dst), std::move(src1), std::move(src2),
                        dimM, dimN, std::move(literalValues)});
}

std::string IRProgram::newTemp() {
    return "t" + std::to_string(m_tempCounter++);
}

void IRProgram::dump() const {
    for (const auto& instr : m_instrs) {
        switch (instr.op) {
            case IROp::SCALAR_DECL:
                std::cerr << "  SCALAR_DECL " << instr.dst << "\n";
                break;
            case IROp::MATRIX_DECL:
                std::cerr << "  MATRIX_DECL " << instr.dst
                          << " [" << instr.dimM << "x" << instr.dimN << "]\n";
                break;
            case IROp::COPY:
                std::cerr << "  " << instr.dst << " = COPY " << instr.src1 << "\n";
                break;
            case IROp::ADD:
                std::cerr << "  " << instr.dst << " = " << instr.src1
                          << " + " << instr.src2;
                if (instr.dimM > 0) std::cerr << "  [matrix " << instr.dimM << "x" << instr.dimN << "]";
                std::cerr << "\n";
                break;
            case IROp::SUB:
                std::cerr << "  " << instr.dst << " = " << instr.src1
                          << " - " << instr.src2;
                if (instr.dimM > 0) std::cerr << "  [matrix " << instr.dimM << "x" << instr.dimN << "]";
                std::cerr << "\n";
                break;
            case IROp::MUL:
                std::cerr << "  " << instr.dst << " = " << instr.src1
                          << " * " << instr.src2;
                if (instr.dimM > 0) std::cerr << "  [matrix " << instr.dimM << "x" << instr.dimN << "]";
                std::cerr << "\n";
                break;
            case IROp::PRINT:
                std::cerr << "  PRINT " << instr.src1 << "\n";
                break;
        }
    }
}
