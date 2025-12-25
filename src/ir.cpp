#include "ir.h"
#include <functional>

// 递归生成 IR：为每个循环分配两个 label（start, end）
IrProgram generateIR(const AstNode &root)
{
    IrProgram prog;
    int labelCounter = 0;

    // 内部递归 lambda
    std::function<void(const AstNode &)> gen = [&](const AstNode &node)
    {
        switch (node.kind)
        {
        case AstKind::MovePtr:
            prog.insts.push_back(IrInst{IrOp::AddPtr, node.arg, 0});
            break;
        case AstKind::AddVal:
            prog.insts.push_back(IrInst{IrOp::AddVal, node.arg, 0});
            break;
        case AstKind::Output:
            prog.insts.push_back(IrInst{IrOp::Output, 0, 0});
            break;
        case AstKind::Input:
            prog.insts.push_back(IrInst{IrOp::Input, 0, 0});
            break;
        case AstKind::Loop:
        {
            int start = labelCounter++;
            int end = labelCounter++;
            prog.insts.push_back(IrInst{IrOp::Label, 0, start});
            prog.insts.push_back(IrInst{IrOp::Jz, end, 0});
            for (const auto &c : node.children)
                gen(*c);
            prog.insts.push_back(IrInst{IrOp::Jnz, start, 0});
            prog.insts.push_back(IrInst{IrOp::Label, 0, end});
            break;
        }
        }
    };

    // 根节点是虚拟 block，展开其 children
    for (const auto &c : root.children)
        gen(*c);
    return prog;
}
