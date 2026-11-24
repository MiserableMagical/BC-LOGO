#pragma once

#include <QVector>

#include "ast.h"
#include "builtins.h"
#include "procedure.h"
#include "token.h"

namespace logo {

class Diagnostics;
class Source;

/// 手写递归下降 + Pratt 表达式。
/// 输入是一段 Source（文件/多行字符串），输出 Program；错误进入 Diagnostics。
class Parser {
public:
    Parser(const Source &source, Diagnostics &diagnostics);
    /// 使用外部过程表，便于 Engine 在多次 runLine 之间保留 arity。
    Parser(const Source &source, Diagnostics &diagnostics, ProcedureTable &procedures);

    Program parseProgram();

    const ProcedureTable &procedures() const { return *procedures_; }

private:
    // token 流
    const Token &peek(int ahead = 0) const;
    const Token &advance();
    bool check(TokenKind kind) const;
    bool match(TokenKind kind);
    void skipNewlines();
    Span spanFrom(const Token &first) const;

    // 语句
    StmtPtr parseInstruction();
    StmtPtr parseCommand(const Token &nameToken, const Builtin &builtin);
    StmtPtr parseProcedureCall(const Token &nameToken, int arity);
    StmtPtr parseDefinition(const Token &toToken);
    StmtPtr parseRepeat(const Token &keyword);
    StmtPtr parseIf(const Token &keyword);
    StmtPtr parseIfElse(const Token &keyword);
    StmtPtr parseWhile(const Token &keyword);
    StmtPtr parseMake(const Token &keyword, bool local);
    StmtPtr parseExpressionStatement(const Token &first);
    StmtList parseBlock();

    // 表达式
    ExprPtr parseExpression(int minBindingPower);
    ExprPtr parseAtom();
    ExprPtr parseParenthesized();
    ExprPtr parseCallExpression(const Token &nameToken, const QString &callee, int arity);

    void synchronize();
    void error(const QString &code, const QString &message, const Span &span);

    const Source &source_;
    Diagnostics &diagnostics_;
    QVector<Token> tokens_;
    int pos_ = 0;
    int definitionDepth_ = 0;
    int parenDepth_ = 0;
    BuiltinTable builtins_;
    ProcedureTable ownedProcedures_;
    ProcedureTable *procedures_ = nullptr;
};

} // namespace logo
