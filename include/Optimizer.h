#pragma once

#include "IR.h"

class Optimizer {
public:
    void optimize(IRProgram& program);

private:
    void foldConstants(IRProgram& program);
    void strengthReduce(IRProgram& program);
};
