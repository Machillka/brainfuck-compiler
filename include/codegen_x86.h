#ifndef BF_CODEGEN_X86_H
#define BF_CODEGEN_X86_H

#include "ir.h"

// JIT 编译器：把 IR 编译为可执行内存中的机器码并返回函数指针
// 返回类型：void(*)(unsigned char* tape)
using BfJitFunc = void (*)(unsigned char*);

BfJitFunc jitCompileX86(const IrProgram& prog);

#endif // BF_CODEGEN_X86_H
