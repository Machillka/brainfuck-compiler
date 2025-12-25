#ifndef BF_AST_H
#define BF_AST_H

#include <memory>
#include <vector>

// AST 节点类型
enum class AstKind
{
    MovePtr, // > 或 <
    AddVal,  // + 或 -
    Output,  // .
    Input,   // ,
    Loop     // [ ... ]
};

// AST 节点结构
struct AstNode
{
    AstKind kind;
    int arg = 0;                                    // 对于 MovePtr/AddVal：步长（可正可负）
    std::vector<std::unique_ptr<AstNode>> children; // 仅 Loop 使用
};

#endif // BF_AST_H
