#include "ast.h"

namespace logo {

namespace {

QString dumpExprList(const std::vector<ExprPtr> &args)
{
    QString out;
    for (const ExprPtr &arg : args)
        out += QLatin1Char(' ') + dump(*arg);
    return out;
}

QString dumpBody(const StmtList &body)
{
    QString out = QStringLiteral("(block");
    for (const StmtPtr &stmt : body)
        out += QLatin1Char(' ') + dump(*stmt);
    out += QLatin1Char(')');
    return out;
}

QString numberText(double value)
{
    return QString::number(value, 'g', 15);
}

} // namespace

QString dump(const Expr &expr)
{
    switch (expr.kind) {
    case ExprKind::Number: {
        const auto &node = static_cast<const NumberExpr &>(expr);
        return numberText(node.value);
    }
    case ExprKind::Word: {
        const auto &node = static_cast<const WordExpr &>(expr);
        return (node.literal ? QStringLiteral("(lit ") : QStringLiteral("(var "))
             + node.text + QLatin1Char(')');
    }
    case ExprKind::Unary: {
        const auto &node = static_cast<const UnaryExpr &>(expr);
        const QString name = (node.op == QStringLiteral("-")) ? QStringLiteral("neg")
                                                              : QStringLiteral("pos");
        return QStringLiteral("(") + name + QLatin1Char(' ') + dump(*node.operand)
             + QLatin1Char(')');
    }
    case ExprKind::Binary: {
        const auto &node = static_cast<const BinaryExpr &>(expr);
        return QStringLiteral("(") + node.op + QLatin1Char(' ') + dump(*node.lhs)
             + QLatin1Char(' ') + dump(*node.rhs) + QLatin1Char(')');
    }
    case ExprKind::Call: {
        const auto &node = static_cast<const CallExpr &>(expr);
        return QStringLiteral("(call ") + node.callee.toLower() + dumpExprList(node.args)
             + QLatin1Char(')');
    }
    }
    return QStringLiteral("<unknown-expr>");
}

QString dump(const Stmt &stmt)
{
    switch (stmt.kind) {
    case StmtKind::Command: {
        const auto &node = static_cast<const CommandStmt &>(stmt);
        return QStringLiteral("(") + node.name.toLower() + dumpExprList(node.args)
             + QLatin1Char(')');
    }
    case StmtKind::Expr: {
        const auto &node = static_cast<const ExprStmt &>(stmt);
        return QStringLiteral("(expr ") + dump(*node.expr) + QLatin1Char(')');
    }
    case StmtKind::Repeat: {
        const auto &node = static_cast<const RepeatStmt &>(stmt);
        return QStringLiteral("(repeat ") + dump(*node.times) + QLatin1Char(' ')
             + dumpBody(node.body) + QLatin1Char(')');
    }
    case StmtKind::If: {
        const auto &node = static_cast<const IfStmt &>(stmt);
        return QStringLiteral("(if ") + dump(*node.cond) + QLatin1Char(' ')
             + dumpBody(node.thenBody) + QLatin1Char(')');
    }
    case StmtKind::IfElse: {
        const auto &node = static_cast<const IfElseStmt &>(stmt);
        return QStringLiteral("(ifelse ") + dump(*node.cond) + QLatin1Char(' ')
             + dumpBody(node.thenBody) + QLatin1Char(' ') + dumpBody(node.elseBody)
             + QLatin1Char(')');
    }
    case StmtKind::While: {
        const auto &node = static_cast<const WhileStmt &>(stmt);
        return QStringLiteral("(while ") + dump(*node.cond) + QLatin1Char(' ')
             + dumpBody(node.body) + QLatin1Char(')');
    }
    case StmtKind::Make: {
        const auto &node = static_cast<const MakeStmt &>(stmt);
        const QString name = node.local ? QStringLiteral("localmake") : QStringLiteral("make");
        return QStringLiteral("(") + name + QLatin1Char(' ') + dump(*node.name)
             + QLatin1Char(' ') + dump(*node.value) + QLatin1Char(')');
    }
    case StmtKind::Stop:
        return QStringLiteral("(stop)");
    case StmtKind::Output: {
        const auto &node = static_cast<const OutputStmt &>(stmt);
        return QStringLiteral("(output ") + dump(*node.value) + QLatin1Char(')');
    }
    case StmtKind::ProcDef: {
        const auto &node = static_cast<const ProcDefStmt &>(stmt);
        QString params = QStringLiteral("(params");
        for (const QString &param : node.params) {
            params += QLatin1Char(' ');
            params += param;
        }
        params += QLatin1Char(')');
        return QStringLiteral("(def ") + node.name.toLower() + QLatin1Char(' ') + params
             + QLatin1Char(' ') + dumpBody(node.body) + QLatin1Char(')');
    }
    }
    return QStringLiteral("<unknown-stmt>");
}

QString dump(const StmtList &statements)
{
    return dumpBody(statements);
}

QString dump(const Program &program)
{
    QString out = QStringLiteral("(program");
    for (const StmtPtr &stmt : program.statements)
        out += QLatin1Char(' ') + dump(*stmt);
    out += QLatin1Char(')');
    return out;
}

} // namespace logo
