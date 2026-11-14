#include "diagnostic.h"

#include <QStringList>

#include "source.h"

namespace logo {

const char *severityName(Severity severity)
{
    switch (severity) {
    case Severity::Error: return "error";
    case Severity::Warning: return "warning";
    case Severity::Note: return "note";
    }
    return "note";
}

void Diagnostics::error(const QString &code, const QString &message, const Span &span)
{
    add(Severity::Error, code, message, span);
}

void Diagnostics::warning(const QString &code, const QString &message, const Span &span)
{
    add(Severity::Warning, code, message, span);
}

void Diagnostics::add(Severity severity, const QString &code, const QString &message, const Span &span)
{
    items_.append(Diagnostic{severity, code, message, span});
}

bool Diagnostics::hasErrors() const
{
    for (const Diagnostic &diagnostic : items_) {
        if (diagnostic.severity == Severity::Error)
            return true;
    }
    return false;
}

QString Diagnostics::format(const Source &source) const
{
    QStringList lines;
    lines.reserve(items_.size());
    for (const Diagnostic &diagnostic : items_)
        lines.append(format(source, diagnostic));
    return lines.join(QLatin1Char('\n'));
}

QString Diagnostics::format(const Source &source, const Diagnostic &diagnostic) const
{
    return QStringLiteral("%1:%2:%3: %4[%5]: %6")
        .arg(source.name())
        .arg(diagnostic.span.line)
        .arg(diagnostic.span.column)
        .arg(QString::fromLatin1(severityName(diagnostic.severity)))
        .arg(diagnostic.code)
        .arg(diagnostic.message);
}

} // namespace logo
