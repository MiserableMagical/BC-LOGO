#pragma once

#include <QString>
#include <QVector>

#include "span.h"

namespace logo {

/// 一段具名源文本（一个文件、一行输入、一个 TO 定义）。
/// 负责 offset 与 line/column 的换算，供词法器和诊断使用。
class Source {
public:
    Source() = default;
    Source(QString name, QString text);

    const QString &name() const { return name_; }
    const QString &text() const { return text_; }
    int length() const { return text_.size(); }
    int lineCount() const { return lineStarts_.size(); }

    /// 把 [offset, offset+length) 映射为带行列的 Span；越界会被夹取。
    Span span(int offset, int length) const;

    /// 1-based 行号对应的文本，不含行尾的 CR/LF。越界返回空串。
    QString lineText(int line) const;

private:
    QString name_;
    QString text_;
    QVector<int> lineStarts_;   // 每行首字符的 offset，至少包含 0
};

} // namespace logo
