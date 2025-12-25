#include "lexer.h"

Lexer::Lexer(const std::string& src) : src_(src), pos_(0)
{
}

Token Lexer::next()
{
    while (true)
    {
        if (pos_ >= static_cast<int>(src_.size()))
        {
            return {TokenKind::END, pos_};
        }
        char c = src_[pos_++];
        switch (c)
        {
        case '>':
            return {TokenKind::GT, pos_ - 1};
        case '<':
            return {TokenKind::LT, pos_ - 1};
        case '+':
            return {TokenKind::PLUS, pos_ - 1};
        case '-':
            return {TokenKind::MINUS, pos_ - 1};
        case '.':
            return {TokenKind::DOT, pos_ - 1};
        case ',':
            return {TokenKind::COMMA, pos_ - 1};
        case '[':
            return {TokenKind::LBRACK, pos_ - 1};
        case ']':
            return {TokenKind::RBRACK, pos_ - 1};
        default:
            // 非指令字符视为注释，跳过
            continue;
        }
    }
}
