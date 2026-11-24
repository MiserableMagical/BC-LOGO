#pragma once

#include <QString>

#include "span.h"

namespace logo {

enum class TokenKind {
    Word,       // 过程名/命令名；未加引号的裸词
    Variable,   // :name；text 不含冒号
    Number,
    Operator,   // + - * / % = < > <= >= & |
    LParen,
    RParen,
    LBracket,
    RBracket,
    Comma,
    Newline,    // 文件模式下的软分隔；REPL 每行单独词法化
    Eof,
    Invalid
};

struct Token {
    TokenKind kind = TokenKind::Invalid;
    QString text;          // 原始拼写，保留大小写
    double number = 0.0;   // 仅 Number 有效
    Span span;
    bool quoted = false;   // "word 形式的词字面量

    bool is(TokenKind other) const { return kind == other; }
};

inline const char *tokenKindName(TokenKind kind)
{
    switch (kind) {
    case TokenKind::Word: return "Word";
    case TokenKind::Variable: return "Variable";
    case TokenKind::Number: return "Number";
    case TokenKind::Operator: return "Operator";
    case TokenKind::LParen: return "LParen";
    case TokenKind::RParen: return "RParen";
    case TokenKind::LBracket: return "LBracket";
    case TokenKind::RBracket: return "RBracket";
    case TokenKind::Comma: return "Comma";
    case TokenKind::Newline: return "Newline";
    case TokenKind::Eof: return "Eof";
    case TokenKind::Invalid: return "Invalid";
    }
    return "Unknown";
}

} // namespace logo
