#pragma once

#include <QString>

#include <memory>
#include <utility>
#include <vector>

#include "span.h"

namespace logo {

// ---------------------------------------------------------------------------
// 表达式
// ---------------------------------------------------------------------------

enum class ExprKind {
    Number,
    Word,
    Unary,
    Binary,
    Call
};

struct Expr {
    Expr(ExprKind kind, Span span = {})
        : kind(kind)
        , span(span) {}
    virtual ~Expr() = default;

    const ExprKind kind;
    Span span;
};

using ExprPtr = std::unique_ptr<Expr>;

struct NumberExpr final : Expr {
    explicit NumberExpr(double value, Span span = {})
        : Expr(ExprKind::Number, span)
        , value(value) {}
    double value;
};

/// 词表达式。literal=true 表示字面量词（如 "RED、"HELLO）；
/// false 表示变量读取（FD X 里的 X）。
struct WordExpr final : Expr {
    WordExpr(QString text, bool literal, Span span = {})
        : Expr(ExprKind::Word, span)
        , text(std::move(text))
        , literal(literal) {}
    QString text;
    bool literal = false;
};

struct UnaryExpr final : Expr {
    UnaryExpr(QString op, ExprPtr operand, Span span = {})
        : Expr(ExprKind::Unary, span)
        , op(std::move(op))
        , operand(std::move(operand)) {}
    QString op; // "+" / "-"
    ExprPtr operand;
};

struct BinaryExpr final : Expr {
    BinaryExpr(QString op, ExprPtr lhs, ExprPtr rhs, Span span = {})
        : Expr(ExprKind::Binary, span)
        , op(std::move(op))
        , lhs(std::move(lhs))
        , rhs(std::move(rhs)) {}
    QString op;
    ExprPtr lhs;
    ExprPtr rhs;
};

/// 前缀运算：SIN 30、COS X、RANDOM 10；后续也用于用户函数。
struct CallExpr final : Expr {
    CallExpr(QString callee, std::vector<ExprPtr> args, Span span = {})
        : Expr(ExprKind::Call, span)
        , callee(std::move(callee))
        , args(std::move(args)) {}
    QString callee;
    std::vector<ExprPtr> args;
};

// ---------------------------------------------------------------------------
// 语句
// ---------------------------------------------------------------------------

enum class StmtKind {
    Command,
    Expr,
    Repeat,
    If,
    IfElse,
    While,
    Make,
    Stop,
    Output,
    ProcDef
};

struct Stmt {
    Stmt(StmtKind kind, Span span = {})
        : kind(kind)
        , span(span) {}
    virtual ~Stmt() = default;

    const StmtKind kind;
    Span span;
};

using StmtPtr = std::unique_ptr<Stmt>;
using StmtList = std::vector<StmtPtr>;

/// 普通命令：FD 100、SETPC RED、PRINT 1+2。
struct CommandStmt final : Stmt {
    CommandStmt(QString name, std::vector<ExprPtr> args, Span span = {})
        : Stmt(StmtKind::Command, span)
        , name(std::move(name))
        , args(std::move(args)) {}
    QString name;
    std::vector<ExprPtr> args;
};

/// 顶层裸表达式，交互模式下回显 "Result is : ..."。
struct ExprStmt final : Stmt {
    explicit ExprStmt(ExprPtr expr, Span span = {})
        : Stmt(StmtKind::Expr, span)
        , expr(std::move(expr)) {}
    ExprPtr expr;
};

struct RepeatStmt final : Stmt {
    RepeatStmt(ExprPtr times, StmtList body, Span span = {})
        : Stmt(StmtKind::Repeat, span)
        , times(std::move(times))
        , body(std::move(body)) {}
    ExprPtr times;
    StmtList body;
};

struct IfStmt final : Stmt {
    IfStmt(ExprPtr cond, StmtList thenBody, Span span = {})
        : Stmt(StmtKind::If, span)
        , cond(std::move(cond))
        , thenBody(std::move(thenBody)) {}
    ExprPtr cond;
    StmtList thenBody;
};

struct IfElseStmt final : Stmt {
    IfElseStmt(ExprPtr cond, StmtList thenBody, StmtList elseBody, Span span = {})
        : Stmt(StmtKind::IfElse, span)
        , cond(std::move(cond))
        , thenBody(std::move(thenBody))
        , elseBody(std::move(elseBody)) {}
    ExprPtr cond;
    StmtList thenBody;
    StmtList elseBody;
};

struct WhileStmt final : Stmt {
    WhileStmt(ExprPtr cond, StmtList body, Span span = {})
        : Stmt(StmtKind::While, span)
        , cond(std::move(cond))
        , body(std::move(body)) {}
    ExprPtr cond;
    StmtList body;
};

struct MakeStmt final : Stmt {
    MakeStmt(ExprPtr name, ExprPtr value, bool local, Span span = {})
        : Stmt(StmtKind::Make, span)
        , name(std::move(name))
        , value(std::move(value))
        , local(local) {}
    ExprPtr name; // 求值结果应为词：MAKE "X 5 / MAKE :NAME 5
    ExprPtr value;
    bool local = false; // true 为 LOCALMAKE
};

struct StopStmt final : Stmt {
    explicit StopStmt(Span span = {})
        : Stmt(StmtKind::Stop, span) {}
};

/// 预留：OUTPUT 的用户函数返回值。
struct OutputStmt final : Stmt {
    explicit OutputStmt(ExprPtr value, Span span = {})
        : Stmt(StmtKind::Output, span)
        , value(std::move(value)) {}
    ExprPtr value;
};

/// TO name [:params...] ... END；body 是已解析的语句列表。
struct ProcDefStmt final : Stmt {
    ProcDefStmt(QString name, std::vector<QString> params, StmtList body, Span span = {})
        : Stmt(StmtKind::ProcDef, span)
        , name(std::move(name))
        , params(std::move(params))
        , body(std::move(body)) {}
    QString name;
    std::vector<QString> params;
    StmtList body;
};

struct Program {
    StmtList statements;
    Span span;
};

// 调试/测试用的规范 S 表达式；不改变大小写。
QString dump(const Expr &expr);
QString dump(const Stmt &stmt);
QString dump(const StmtList &statements);
QString dump(const Program &program);

} // namespace logo
