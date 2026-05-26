#include "Optimizer.h"

#include <cctype>
#include <string>

static bool isNumber(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    for (size_t i = start; i < s.size(); i++) {
        if (!std::isdigit(s[i])) return false;
    }
    return !s.empty() && start < s.size();
}

void Optimizer::optimize(IRProgram& program) {
    foldConstants(program);
    strengthReduce(program);
}

void Optimizer::foldConstants(IRProgram& program) {
    auto& instrs = program.instructions();

    for (size_t i = 0; i < instrs.size(); i++) {
        auto& instr = instrs[i];

        if (instr.op != IROp::ADD && instr.op != IROp::SUB && instr.op != IROp::MUL) {
            continue;
        }

        if (instr.dimM > 0 || instr.dimN > 0) {
            continue;
        }

        if (!isNumber(instr.src1) || !isNumber(instr.src2)) {
            continue;
        }

        int a = std::stoi(instr.src1);
        int b = std::stoi(instr.src2);
        int result = 0;

        switch (instr.op) {
            case IROp::ADD: result = a + b; break;
            case IROp::SUB: result = a - b; break;
            case IROp::MUL: result = a * b; break;
            default: break;
        }

        std::string dst = instr.dst;
        instrs[i] = {IROp::COPY, dst, std::to_string(result), "", 0, 0, {}};
    }
}

void Optimizer::strengthReduce(IRProgram& program) {
    auto& instrs = program.instructions();

    for (size_t i = 0; i < instrs.size(); i++) {
        auto& instr = instrs[i];

        if (instr.dimM > 0 || instr.dimN > 0) {
            continue;
        }

        bool src1IsNum = isNumber(instr.src1);
        bool src2IsNum = isNumber(instr.src2);

        switch (instr.op) {
            case IROp::ADD:
            case IROp::SUB:
                if (instr.op == IROp::ADD) {
                    if (src2IsNum && std::stoi(instr.src2) == 0) {
                        instrs[i] = {IROp::COPY, instr.dst, instr.src1, "", 0, 0, {}};
                    } else if (src1IsNum && std::stoi(instr.src1) == 0) {
                        instrs[i] = {IROp::COPY, instr.dst, instr.src2, "", 0, 0, {}};
                    }
                }
                break;
            case IROp::MUL:
                if (src2IsNum && std::stoi(instr.src2) == 1) {
                    instrs[i] = {IROp::COPY, instr.dst, instr.src1, "", 0, 0, {}};
                } else if (src1IsNum && std::stoi(instr.src1) == 1) {
                    instrs[i] = {IROp::COPY, instr.dst, instr.src2, "", 0, 0, {}};
                } else if (src2IsNum && std::stoi(instr.src2) == 0) {
                    instrs[i] = {IROp::COPY, instr.dst, "0", "", 0, 0, {}};
                } else if (src1IsNum && std::stoi(instr.src1) == 0) {
                    instrs[i] = {IROp::COPY, instr.dst, "0", "", 0, 0, {}};
                }
                break;
            default:
                break;
        }
    }
}
