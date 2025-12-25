#include "optimizer.h"
#include <vector>

// 合并连续的 AddPtr / AddVal 指令（peephole）
void optimizeIr(IrProgram& prog)
{
    std::vector<IrInst> out;
    out.reserve(prog.insts.size());

    for (size_t i = 0; i < prog.insts.size();)
    {
        IrInst inst = prog.insts[i];
        if (inst.op == IrOp::AddPtr || inst.op == IrOp::AddVal)
        {
            int sum = inst.arg;
            size_t j = i + 1;
            while (j < prog.insts.size() && prog.insts[j].op == inst.op)
            {
                sum += prog.insts[j].arg;
                ++j;
            }
            if (sum != 0)
            {
                out.push_back(IrInst{inst.op, sum, 0});
            }
            i = j;
        }
        else
        {
            out.push_back(inst);
            ++i;
        }
    }

    prog.insts.swap(out);
}
