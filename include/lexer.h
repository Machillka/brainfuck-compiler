#ifndef BF_LEXER_H
#define BF_LEXER_H

#include <string>

// Token 类型：对应 Brainfuck 的 8 个指令与结束符
enum class TokenKind
{
    GT,
    LT,
    PLUS,
    MINUS,
    DOT,
    COMMA,
    LBRACK,
    RBRACK,
    END
};

struct Token
{
    TokenKind kind;
    int pos; // 源码位置（用于错误信息）
};

// 词法分析器：把源代码字符流转换为 Token 流
class Lexer
{
  public:
    explicit Lexer(const std::string& src);
    Token next(); // 返回下一个 token

  private:
    std::string src_;
    int pos_;
};

#endif // BF_LEXER_H
