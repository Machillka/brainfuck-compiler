#ifndef BF_OPTIMIZER_H
#define BF_OPTIMIZER_H

#include "ir.h"

// 对 IR 做简单 peephole 优化（合并连续的 AddPtr/AddVal）
void optimizeIr(IrProgram& prog);

#endif // BF_OPTIMIZER_H
