#ifndef BF_IR_H
#define BF_IR_H

#include "ast.h"
#include <vector>

// 线性 IR 指令集（简洁、接近机器）
enum class IrOp
{
    AddPtr, // ptr += arg
    AddVal, // *ptr += arg
    Output, // putchar(*ptr)
    Input,  // *ptr = getchar()
    Jz,     // if (*ptr == 0) jump to label arg
    Jnz,    // if (*ptr != 0) jump to label arg
    Label   // label with id
};

struct IrInst
{
    IrOp op;
    int arg;      // AddPtr/AddVal: 步长；Jz/Jnz: 目标 label id
    int label_id; // Label: 标号编号
};

struct IrProgram
{
    std::vector<IrInst> insts;
};

IrProgram generateIR(const AstNode& root);

#endif // BF_IR_H
