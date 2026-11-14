#pragma once

#include <QString>
#include <QVector>

#include "span.h"

namespace logo {

class Source;

enum class Severity { Error, Warning, Note };

struct Diagnostic {
    Severity severity = Severity::Error;
    QString code;
    QString message;
    Span span;
};

const char *severityName(Severity severity);

/// 收集诊断，而不是在出错点直接写 UI。
class Diagnostics {
public:
    void error(const QString &code, const QString &message, const Span &span);
    void warning(const QString &code, const QString &message, const Span &span);
    void add(Severity severity, const QString &code, const QString &message, const Span &span);

    bool isEmpty() const { return items_.isEmpty(); }
    bool hasErrors() const;
    int count() const { return items_.size(); }
    void clear() { items_.clear(); }

    const QVector<Diagnostic> &all() const { return items_; }

    /// "main.lgo:3:9: error[E1001]: unexpected character '@'"
    QString format(const Source &source) const;
    QString format(const Source &source, const Diagnostic &diagnostic) const;

private:
    QVector<Diagnostic> items_;
};

} // namespace logo
