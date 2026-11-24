#include "interpreter.h"

#include <cmath>
#include <utility>

namespace logo {

namespace {

constexpr int kMaxIterations = 1000000;
constexpr int kMaxCallDepth = 1000;
constexpr double kPi = 3.14159265358979323846;

struct FrameGuard {
    Environment &environment;
    ~FrameGuard() { environment.popFrame(); }
};

struct DepthGuard {
    int &depth;
    explicit DepthGuard(int &value)
        : depth(value) { ++depth; }
    ~DepthGuard() { --depth; }
};

} // namespace

Interpreter::Interpreter(Console &console, Plotter &plotter)
    : console_(console)
    , plotter_(plotter)
    , random_(std::random_device{}())
{
}

void Interpreter::run(const Program &program)
{
    try {
        for (const StmtPtr &stmt : program.statements)
            execute(*stmt);
    } catch (const StopSignal &) {
        // 顶层 STOP：正常结束程序。
    } catch (const OutputSignal &) {
        throw RuntimeError{QStringLiteral("OUTPUT can only be used inside a procedure"), Span{}};
    }
}

void Interpreter::executeBlock(const StmtList &body)
{
    for (const StmtPtr &stmt : body)
        execute(*stmt);
}

void Interpreter::execute(const Stmt &stmt)
{
    switch (stmt.kind) {
    case StmtKind::ProcDef: {
        const auto &procedure = static_cast<const ProcDefStmt &>(stmt);
        registerProcedure(procedure);
        return;
    }
    case StmtKind::Command: {
        const auto &command = static_cast<const CommandStmt &>(stmt);

        if (procedures_.contains(command.name)) {
            std::vector<Value> args;
            args.reserve(command.args.size());
            for (const ExprPtr &expr : command.args)
                args.push_back(evaluate(*expr));

            const auto result = invoke(*procedures_.value(command.name), args);
            if (result)
                console_.print(result->toString());
            return;
        }
        executeCommand(command);
        return;
    }
    case StmtKind::Expr: {
        const auto &expression = static_cast<const ExprStmt &>(stmt);
        console_.print(evaluate(*expression.expr).toString());
        return;
    }
    case StmtKind::Repeat: {
        const auto &repeat = static_cast<const RepeatStmt &>(stmt);
        const double raw = asNumber(evaluate(*repeat.times), repeat.times->span,
                                    QStringLiteral("REPEAT"));
        if (raw > kMaxIterations) {
            throw RuntimeError{QStringLiteral("REPEAT count is too large"), repeat.times->span};
        }
        const int count = raw <= 0.0 ? 0 : static_cast<int>(raw);
        for (int i = 0; i < count; ++i)
            executeBlock(repeat.body);
        return;
    }
    case StmtKind::If: {
        const auto &branch = static_cast<const IfStmt &>(stmt);
        if (asNumber(evaluate(*branch.cond), branch.cond->span, QStringLiteral("IF")) != 0.0)
            executeBlock(branch.thenBody);
        return;
    }
    case StmtKind::IfElse: {
        const auto &branch = static_cast<const IfElseStmt &>(stmt);
        if (asNumber(evaluate(*branch.cond), branch.cond->span, QStringLiteral("IFELSE")) != 0.0)
            executeBlock(branch.thenBody);
        else
            executeBlock(branch.elseBody);
        return;
    }
    case StmtKind::While: {
        const auto &loop = static_cast<const WhileStmt &>(stmt);
        int iterations = 0;
        while (asNumber(evaluate(*loop.cond), loop.cond->span, QStringLiteral("WHILE")) != 0.0) {
            if (++iterations > kMaxIterations) {
                throw RuntimeError{QStringLiteral("WHILE exceeded the iteration limit"),
                                   loop.cond->span};
            }
            executeBlock(loop.body);
        }
        return;
    }
    case StmtKind::Make: {
        const auto &make = static_cast<const MakeStmt &>(stmt);
        const Value name = evaluate(*make.name);
        if (!name.isWord()) {
            throw RuntimeError{QStringLiteral("MAKE needs a word as variable name"),
                               make.name->span};
        }
        const Value value = evaluate(*make.value);
        if (make.local)
            environment_.setLocal(name.word(), value);
        else
            environment_.setNearestOrGlobal(name.word(), value);
        return;
    }
    case StmtKind::Stop:
        throw StopSignal{};
    case StmtKind::Output: {
        const auto &output = static_cast<const OutputStmt &>(stmt);
        throw OutputSignal{evaluate(*output.value)};
    }
    }
}

void Interpreter::executeCommand(const CommandStmt &command)
{
    const QString &name = command.name;

    if (name == QLatin1String("print")) {
        console_.print(argValue(command, 0).toString());
        return;
    }
    if (name == QLatin1String("fd")) {
        plotter_.forward(argNumber(command, 0));
        return;
    }
    if (name == QLatin1String("bk")) {
        plotter_.backward(argNumber(command, 0));
        return;
    }
    if (name == QLatin1String("lt")) {
        plotter_.turnLeft(argNumber(command, 0));
        return;
    }
    if (name == QLatin1String("rt")) {
        plotter_.turnRight(argNumber(command, 0));
        return;
    }
    if (name == QLatin1String("cs")) {
        plotter_.clearScreen();
        return;
    }
    if (name == QLatin1String("clean")) {
        plotter_.clean();
        return;
    }
    if (name == QLatin1String("home")) {
        plotter_.home();
        return;
    }
    if (name == QLatin1String("st")) {
        plotter_.showTurtle();
        return;
    }
    if (name == QLatin1String("ht")) {
        plotter_.hideTurtle();
        return;
    }
    if (name == QLatin1String("pu")) {
        plotter_.penUp();
        return;
    }
    if (name == QLatin1String("pd")) {
        plotter_.penDown();
        return;
    }
    if (name == QLatin1String("setw")) {
        plotter_.setPenWidth(argNumber(command, 0));
        return;
    }
    if (name == QLatin1String("setpc")) {
        plotter_.setPenColor(argText(command, 0));
        return;
    }
    if (name == QLatin1String("setbg")) {
        plotter_.setBackground(argText(command, 0));
        return;
    }
    if (name == QLatin1String("setpcdec")) {
        const int red = qBound(0, static_cast<int>(argNumber(command, 0)), 255);
        const int green = qBound(0, static_cast<int>(argNumber(command, 1)), 255);
        const int blue = qBound(0, static_cast<int>(argNumber(command, 2)), 255);
        plotter_.setPenColorRgb(red, green, blue);
        return;
    }
    if (name == QLatin1String("setx")) {
        plotter_.setX(argNumber(command, 0));
        return;
    }
    if (name == QLatin1String("sety")) {
        plotter_.setY(argNumber(command, 0));
        return;
    }
    if (name == QLatin1String("setxy")) {
        plotter_.setXY(argNumber(command, 0), argNumber(command, 1));
        return;
    }
    if (name == QLatin1String("wait")) {
        return;
    }

    throw RuntimeError{QStringLiteral("unknown command '%1'").arg(name), command.span};
}

Value Interpreter::evaluate(const Expr &expr)
{
    switch (expr.kind) {
    case ExprKind::Number:
        return Value::number(static_cast<const NumberExpr &>(expr).value);
    case ExprKind::Word: {
        const auto &word = static_cast<const WordExpr &>(expr);
        if (word.literal)
            return Value::word(word.text);

        const auto value = environment_.lookup(word.text);
        if (!value) {
            throw RuntimeError{QStringLiteral("variable '%1' has no value").arg(word.text),
                               expr.span};
        }
        return *value;
    }
    case ExprKind::Unary: {
        const auto &unary = static_cast<const UnaryExpr &>(expr);
        const double value = asNumber(evaluate(*unary.operand), unary.operand->span,
                                      QStringLiteral("unary %1").arg(unary.op));
        return Value::number(unary.op == QLatin1String("-") ? -value : value);
    }
    case ExprKind::Binary: {
        const auto &binary = static_cast<const BinaryExpr &>(expr);
        const Value lhs = evaluate(*binary.lhs);
        const Value rhs = evaluate(*binary.rhs);
        const QString &op = binary.op;

        if (op == QLatin1String("="))
            return Value::number(lhs.equals(rhs) ? 1.0 : 0.0);

        if (op == QLatin1String("&")) {
            const double l = asNumber(lhs, binary.lhs->span, QStringLiteral("AND"));
            const double r = asNumber(rhs, binary.rhs->span, QStringLiteral("AND"));
            return Value::number((l != 0.0 && r != 0.0) ? 1.0 : 0.0);
        }
        if (op == QLatin1String("|")) {
            const double l = asNumber(lhs, binary.lhs->span, QStringLiteral("OR"));
            const double r = asNumber(rhs, binary.rhs->span, QStringLiteral("OR"));
            return Value::number((l != 0.0 || r != 0.0) ? 1.0 : 0.0);
        }

        if (op == QLatin1String("<") || op == QLatin1String(">")
            || op == QLatin1String("<=") || op == QLatin1String(">=")) {
            const double l = asNumber(lhs, binary.lhs->span, op);
            const double r = asNumber(rhs, binary.rhs->span, op);
            bool result = false;
            if (op == QLatin1String("<"))
                result = l < r;
            else if (op == QLatin1String(">"))
                result = l > r;
            else if (op == QLatin1String("<="))
                result = l <= r;
            else
                result = l >= r;
            return Value::number(result ? 1.0 : 0.0);
        }

        const double l = asNumber(lhs, binary.lhs->span, op);
        const double r = asNumber(rhs, binary.rhs->span, op);

        if (op == QLatin1String("+"))
            return Value::number(l + r);
        if (op == QLatin1String("-"))
            return Value::number(l - r);
        if (op == QLatin1String("*"))
            return Value::number(l * r);
        if (op == QLatin1String("/")) {
            if (r == 0.0)
                throw RuntimeError{QStringLiteral("division by zero"), expr.span};
            return Value::number(l / r);
        }
        if (op == QLatin1String("%")) {
            if (r == 0.0)
                throw RuntimeError{QStringLiteral("division by zero"), expr.span};
            return Value::number(std::fmod(l, r));
        }

        throw RuntimeError{QStringLiteral("unknown operator '%1'").arg(op), expr.span};
    }
    case ExprKind::Call:
        return evaluateCall(static_cast<const CallExpr &>(expr));
    }
    throw RuntimeError{QStringLiteral("unknown expression"), expr.span};
}

Value Interpreter::evaluateCall(const CallExpr &call)
{
    if (call.callee == QLatin1String("sin") || call.callee == QLatin1String("cos")) {
        if (call.args.size() != 1) {
            throw RuntimeError{QStringLiteral("'%1' needs one input").arg(call.callee),
                               call.span};
        }
        const double degrees = asNumber(evaluate(*call.args.at(0)),
                                        call.args.at(0)->span, call.callee);
        const double radians = degrees * kPi / 180.0;
        return Value::number(call.callee == QLatin1String("sin") ? std::sin(radians)
                                                                 : std::cos(radians));
    }

    if (call.callee == QLatin1String("random")) {
        if (call.args.size() != 1) {
            throw RuntimeError{QStringLiteral("'random' needs one input"), call.span};
        }
        const double limit = asNumber(evaluate(*call.args.at(0)),
                                      call.args.at(0)->span, QStringLiteral("RANDOM"));
        const int upper = static_cast<int>(std::floor(limit));
        if (upper <= 1)
            return Value::number(1);
        std::uniform_int_distribution<int> distribution(1, upper);
        return Value::number(distribution(random_));
    }

    if (builtins_.contains(call.callee)) {
        throw RuntimeError{QStringLiteral("'%1' is a command, not an operation")
                               .arg(call.callee),
                           call.span};
    }

    const auto procedure = procedures_.constFind(call.callee);
    if (procedure == procedures_.constEnd()) {
        throw RuntimeError{QStringLiteral("unknown procedure '%1'").arg(call.callee),
                           call.span};
    }

    std::vector<Value> args;
    args.reserve(call.args.size());
    for (const ExprPtr &arg : call.args)
        args.push_back(evaluate(*arg));

    const auto result = invoke(**procedure, args);
    if (!result) {
        throw RuntimeError{QStringLiteral("procedure '%1' didn't output").arg(call.callee),
                           call.span};
    }
    return *result;
}

std::optional<Value> Interpreter::invoke(const ProcDefStmt &procedure,
                                         const std::vector<Value> &args)
{
    if (procedure.params.size() != args.size()) {
        throw RuntimeError{QStringLiteral("'%1' needs %2 input(s)")
                               .arg(procedure.name)
                               .arg(procedure.params.size()),
                           procedure.span};
    }
    if (callDepth_ >= kMaxCallDepth) {
        throw RuntimeError{QStringLiteral("too many recursive calls"), procedure.span};
    }

    QHash<QString, Value> locals;
    for (int i = 0; i < static_cast<int>(procedure.params.size()); ++i)
        locals.insert(procedure.params.at(i).toLower(), args.at(i));

    environment_.pushFrame(std::move(locals));
    FrameGuard frameGuard{environment_};
    DepthGuard depthGuard{callDepth_};

    try {
        executeBlock(procedure.body);
    } catch (const OutputSignal &output) {
        return output.value;
    } catch (const StopSignal &) {
        return std::nullopt;
    }
    return std::nullopt;
}

Value Interpreter::argValue(const CommandStmt &command, int index)
{
    if (index < 0 || index >= static_cast<int>(command.args.size())) {
        throw RuntimeError{QStringLiteral("'%1' is missing an input").arg(command.name),
                           command.span};
    }
    return evaluate(*command.args.at(index));
}

double Interpreter::argNumber(const CommandStmt &command, int index)
{
    const ExprPtr &expr = command.args.at(index);
    return asNumber(argValue(command, index), expr->span, command.name);
}

QString Interpreter::argText(const CommandStmt &command, int index)
{
    return argValue(command, index).toString();
}

double Interpreter::asNumber(const Value &value, const Span &span,
                             const QString &context) const
{
    if (!value.isNumber()) {
        throw RuntimeError{QStringLiteral("%1 needs a number, got '%2'")
                               .arg(context, value.toString()),
                           span};
    }
    return value.number();
}

} // namespace logo
