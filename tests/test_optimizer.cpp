#include "doctest.h"
#include "IR.h"
#include "Optimizer.h"

TEST_CASE("Optimizer - constant folding ADD") {
    IRProgram ir;
    ir.emit(IROp::SCALAR_DECL, "t0");
    ir.emit(IROp::SCALAR_DECL, "x");
    ir.emit(IROp::ADD, "x", "5", "3");

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    REQUIRE(instrs.size() == 3);
    CHECK(instrs[2].op == IROp::COPY);
    CHECK(instrs[2].src1 == "8");
}

TEST_CASE("Optimizer - constant folding MUL") {
    IRProgram ir;
    ir.emit(IROp::SCALAR_DECL, "t0");
    ir.emit(IROp::SCALAR_DECL, "x");
    ir.emit(IROp::MUL, "x", "6", "7");

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    CHECK(instrs[2].op == IROp::COPY);
    CHECK(instrs[2].src1 == "42");
}

TEST_CASE("Optimizer - constant folding SUB") {
    IRProgram ir;
    ir.emit(IROp::SCALAR_DECL, "x");
    ir.emit(IROp::SUB, "x", "10", "3");

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    CHECK(instrs[1].op == IROp::COPY);
    CHECK(instrs[1].src1 == "7");
}

TEST_CASE("Optimizer - does NOT fold non-constant") {
    IRProgram ir;
    ir.emit(IROp::SCALAR_DECL, "x");
    ir.emit(IROp::SCALAR_DECL, "y");
    ir.emit(IROp::ADD, "z", "x", "y");

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    CHECK(instrs[2].op == IROp::ADD);
}

TEST_CASE("Optimizer - does NOT fold matrix ops") {
    IRProgram ir;
    ir.emit(IROp::MATRIX_DECL, "A", "", "", 2, 2);
    ir.emit(IROp::MATRIX_DECL, "B", "", "", 2, 2);
    ir.emit(IROp::ADD, "C", "A", "B", 2, 2);

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    CHECK(instrs[2].op == IROp::ADD);
    CHECK(instrs[2].dimM == 2);
}

TEST_CASE("Optimizer - strength reduce MUL by 1") {
    IRProgram ir;
    ir.emit(IROp::SCALAR_DECL, "x");
    ir.emit(IROp::SCALAR_DECL, "y");
    ir.emit(IROp::MUL, "z", "x", "1");

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    CHECK(instrs[2].op == IROp::COPY);
    CHECK(instrs[2].src1 == "x");
}

TEST_CASE("Optimizer - strength reduce MUL by 0") {
    IRProgram ir;
    ir.emit(IROp::SCALAR_DECL, "t0");
    ir.emit(IROp::SCALAR_DECL, "x");
    ir.emit(IROp::MUL, "x", "t0", "0");

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    CHECK(instrs[2].op == IROp::COPY);
    CHECK(instrs[2].src1 == "0");
}

TEST_CASE("Optimizer - strength reduce ADD 0") {
    IRProgram ir;
    ir.emit(IROp::SCALAR_DECL, "x");
    ir.emit(IROp::ADD, "y", "x", "0");

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    CHECK(instrs[1].op == IROp::COPY);
    CHECK(instrs[1].src1 == "x");
}

TEST_CASE("Optimizer - does NOT reduce non-scalar ops") {
    IRProgram ir;
    ir.emit(IROp::MATRIX_DECL, "A", "", "", 2, 2);
    ir.emit(IROp::MATRIX_DECL, "B", "", "", 2, 2);
    ir.emit(IROp::MUL, "C", "A", "B", 2, 2);

    Optimizer opt;
    opt.optimize(ir);

    const auto& instrs = ir.instructions();
    CHECK(instrs[2].op == IROp::MUL);
}
