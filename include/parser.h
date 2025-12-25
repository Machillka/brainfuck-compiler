#ifndef BF_PARSER_H
#define BF_PARSER_H

#include "ast.h"
#include "lexer.h"
#include <memory>

// 递归下降解析器：从 token 流构建 AST
class Parser
{
  public:
    explicit Parser(const std::string& src);
    std::unique_ptr<AstNode> parseProgram(); // 返回根节点（虚拟 block）

  private:
    Token cur_;
    Lexer lexer_;
    void advance();
    std::unique_ptr<AstNode> parseStmt();
    std::unique_ptr<AstNode> parseLoop();
};

#endif // BF_PARSER_H
