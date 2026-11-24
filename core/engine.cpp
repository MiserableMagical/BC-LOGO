#include "engine.h"

#include "diagnostic.h"
#include "lexer.h"
#include "parser.h"
#include "source.h"

#include <QRegularExpression>

#include <utility>

namespace logo {

Engine::Engine(Console &console, Plotter &plotter)
    : console_(console)
    , interpreter_(console, plotter)
{
}

void Engine::runLine(const QString &line)
{
    if (defining_) {
        definitionLines_.append(line);
        if (hasEndToken(line)) {
            const QString text = definitionLines_.join(QLatin1Char('\n'));
            definitionLines_.clear();
            defining_ = false;

            const Source source(QStringLiteral("<definition>"), text);
            runSource(source);
        }
        return;
    }

    if (line.trimmed().isEmpty())
        return;

    if (startsDefinition(line)) {
        if (hasEndToken(line)) {
            const Source source(QStringLiteral("<definition>"), line);
            runSource(source);
        } else {
            defining_ = true;
            definitionLines_.append(line);
        }
        return;
    }

    const Source source(QStringLiteral("<listener>"), line);
    Diagnostics diagnostics;
    Parser parser(source, diagnostics, procedures_);
    Program program = parser.parseProgram();

    reportDiagnostics(source, diagnostics);
    if (diagnostics.hasErrors())
        return;

    executeProgram(std::move(program), source);
}

void Engine::runSource(const Source &source)
{
    defining_ = false;
    definitionLines_.clear();

    Diagnostics diagnostics;
    Parser parser(source, diagnostics, procedures_);
    Program program = parser.parseProgram();

    reportDiagnostics(source, diagnostics);
    if (diagnostics.hasErrors())
        return;

    executeProgram(std::move(program), source);
}

void Engine::executeProgram(Program program, const Source &source)
{
    Program batch;

    const auto flushBatch = [&]() {
        if (batch.statements.empty())
            return;
        Program runnable = std::move(batch);
        batch = Program();
        try {
            interpreter_.run(runnable);
        } catch (const RuntimeError &error) {
            reportRuntimeError(error);
        }
    };

    for (StmtPtr &stmt : program.statements) {
        if (!stmt)
            continue;

        if (stmt->kind != StmtKind::ProcDef) {
            batch.statements.push_back(std::move(stmt));
            continue;
        }

        // 先执行定义之前的语句，保证“定义 -> 调用 -> 重定义 -> 调用”的顺序。
        flushBatch();

        auto &procedure = static_cast<ProcDefStmt &>(*stmt);
        const QString name = procedure.name;

        const int offset = qBound(0, procedure.span.offset, source.length());
        const int length = qBound(0, procedure.span.length, source.length() - offset);
        const QString definitionText = source.text().mid(offset, length);
        const bool existed = definitions_.contains(name);

        definitions_.insert(name, definitionText);
        interpreter_.registerProcedure(procedure);

        Program storage;
        storage.statements.push_back(std::move(stmt));
        programs_.push_back(std::move(storage));

        console_.print(name.toUpper()
                       + (existed ? QStringLiteral(" redefined.") : QStringLiteral(" defined.")));
    }

    flushBatch();
}

void Engine::reportDiagnostics(const Source &source, const Diagnostics &diagnostics)
{
    for (const Diagnostic &diagnostic : diagnostics.all())
        console_.print(diagnostics.format(source, diagnostic));
}

void Engine::reportRuntimeError(const RuntimeError &error)
{
    if (error.span.line > 0) {
        console_.print(QStringLiteral("Error at %1:%2: %3")
                           .arg(error.span.line)
                           .arg(error.span.column)
                           .arg(error.message));
    } else {
        console_.print(QStringLiteral("Error: %1").arg(error.message));
    }
}

bool Engine::startsDefinition(const QString &line) const
{
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty())
        return false;

    const QStringList parts =
        trimmed.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    return !parts.isEmpty() && parts.first().compare(QStringLiteral("to"), Qt::CaseInsensitive) == 0;
}

bool Engine::hasEndToken(const QString &line) const
{
    const Source source(QStringLiteral("<line>"), line);
    Diagnostics ignored;
    Lexer lexer(source, &ignored);

    for (const Token &token : lexer.tokenizeAll()) {
        if (token.kind == TokenKind::Word && !token.quoted
            && token.text.compare(QStringLiteral("end"), Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace logo
