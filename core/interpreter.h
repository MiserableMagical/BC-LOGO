#pragma once

#include <QHash>
#include <QString>

#include <optional>
#include <random>
#include <vector>

#include "ast.h"
#include "builtins.h"
#include "environment.h"
#include "runtime.h"
#include "value.h"

namespace logo {

/// STOP：结束当前过程，不返回值。
struct StopSignal {
};

/// OUTPUT：结束当前过程，并把 value 交给调用者。
struct OutputSignal {
    Value value;
};

struct RuntimeError {
    QString message;
    Span span;
};

/// 树遍历解释器。程序/过程体的生命周期由调用者保证。
class Interpreter {
public:
    Interpreter(Console &console, Plotter &plotter);

    /// 执行 Program；遇到顶层 STOP 正常结束，RuntimeError 向上抛。
    void run(const Program &program);

    const Environment &environment() const { return environment_; }
    const QHash<QString, const ProcDefStmt *> &procedures() const { return procedures_; }

    /// Engine 持久化定义时使用；procedure 必须比 Interpreter 活得更久。
    void registerProcedure(const ProcDefStmt &procedure)
    {
        procedures_.insert(procedure.name.toLower(), &procedure);
    }

private:
    void execute(const Stmt &stmt);
    void executeBlock(const StmtList &body);
    void executeCommand(const CommandStmt &command);
    Value evaluate(const Expr &expr);
    Value evaluateCall(const CallExpr &call);
    std::optional<Value> invoke(const ProcDefStmt &procedure, const std::vector<Value> &args);

    Value argValue(const CommandStmt &command, int index);
    double argNumber(const CommandStmt &command, int index);
    QString argText(const CommandStmt &command, int index);
    double asNumber(const Value &value, const Span &span, const QString &context) const;

    Console &console_;
    Plotter &plotter_;
    Environment environment_;
    QHash<QString, const ProcDefStmt *> procedures_;
    BuiltinTable builtins_;
    std::mt19937 random_;
    int callDepth_ = 0;
};

} // namespace logo
