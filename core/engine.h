#pragma once

#include <QMap>
#include <QString>
#include <QStringList>

#include <vector>

#include "ast.h"
#include "interpreter.h"
#include "procedure.h"
#include "runtime.h"

namespace logo {

class Diagnostics;
class Source;

/// GUI/CLI 门面：把逐行输入与整文件输入接到 Parser + Interpreter，
/// 并在多次 runLine 之间保持过程表、变量环境和定义体生命周期。
class Engine {
public:
    Engine(Console &console, Plotter &plotter);

    /// 交互式一行。TO…END 会缓冲到出现 END 再解析/执行。
    void runLine(const QString &line);

    /// 整段源文本（文件模式）。
    void runSource(const Source &source);

    bool isDefining() const { return defining_; }

    /// 定义名（小写）→ 原始定义文本，供保存使用。
    const QMap<QString, QString> &definitions() const { return definitions_; }

    const Environment &environment() const { return interpreter_.environment(); }

private:
    void executeProgram(Program program, const Source &source);
    void reportDiagnostics(const Source &source, const Diagnostics &diagnostics);
    void reportRuntimeError(const RuntimeError &error);
    bool startsDefinition(const QString &line) const;
    bool hasEndToken(const QString &line) const;

    Console &console_;
    Interpreter interpreter_;
    ProcedureTable procedures_;
    std::vector<Program> programs_; // 持有 ProcDefStmt，保证 Interpreter 里的指针有效
    QMap<QString, QString> definitions_;
    QStringList definitionLines_;
    bool defining_ = false;
};

} // namespace logo
