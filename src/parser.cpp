#include "parser.h"
#include <cstdlib>
#include <iostream>

Parser::Parser(const std::string& src) : lexer_(src)
{
    advance();
}

void Parser::advance()
{
    cur_ = lexer_.next();
}

std::unique_ptr<AstNode> Parser::parseProgram()
{
    // 用一个虚拟 Loop 作为根节点，表示语句序列
    auto root = std::make_unique<AstNode>();
    root->kind = AstKind::Loop;
    while (cur_.kind != TokenKind::END)
    {
        auto s = parseStmt();
        if (s)
            root->children.push_back(std::move(s));
    }
    return root;
}

std::unique_ptr<AstNode> Parser::parseStmt()
{
    using TK = TokenKind;
    using AK = AstKind;

    switch (cur_.kind)
    {
    case TK::GT:
    {
        auto n = std::make_unique<AstNode>();
        n->kind = AK::MovePtr;
        n->arg = +1;
        advance();
        return n;
    }
    case TK::LT:
    {
        auto n = std::make_unique<AstNode>();
        n->kind = AK::MovePtr;
        n->arg = -1;
        advance();
        return n;
    }
    case TK::PLUS:
    {
        auto n = std::make_unique<AstNode>();
        n->kind = AK::AddVal;
        n->arg = +1;
        advance();
        return n;
    }
    case TK::MINUS:
    {
        auto n = std::make_unique<AstNode>();
        n->kind = AK::AddVal;
        n->arg = -1;
        advance();
        return n;
    }
    case TK::DOT:
    {
        auto n = std::make_unique<AstNode>();
        n->kind = AK::Output;
        advance();
        return n;
    }
    case TK::COMMA:
    {
        auto n = std::make_unique<AstNode>();
        n->kind = AK::Input;
        advance();
        return n;
    }
    case TK::LBRACK:
        return parseLoop();
    default:
        return nullptr;
    }
}

std::unique_ptr<AstNode> Parser::parseLoop()
{
    advance(); // 吃掉 '['
    auto loop = std::make_unique<AstNode>();
    loop->kind = AstKind::Loop;

    while (cur_.kind != TokenKind::RBRACK && cur_.kind != TokenKind::END)
    {
        auto s = parseStmt();
        if (s)
            loop->children.push_back(std::move(s));
    }

    if (cur_.kind != TokenKind::RBRACK)
    {
        std::cerr << "Syntax error: unmatched '['\n";
        std::exit(1);
    }
    advance(); // 吃掉 ']'
    return loop;
}
