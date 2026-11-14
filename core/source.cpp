#include "source.h"

#include <algorithm>
#include <utility>

namespace logo {

Source::Source(QString name, QString text)
    : name_(std::move(name))
    , text_(std::move(text))
{
    lineStarts_.append(0);
    for (int i = 0; i < text_.size(); ++i) {
        if (text_.at(i) == QLatin1Char('\n'))
            lineStarts_.append(i + 1);
    }
}

Span Source::span(int offset, int length) const
{
    offset = qBound(0, offset, text_.size());
    length = qBound(0, length, text_.size() - offset);

    const auto it = std::upper_bound(lineStarts_.cbegin(), lineStarts_.cend(), offset);
    const int line = static_cast<int>(it - lineStarts_.cbegin());
    const int start = lineStarts_.at(line - 1);

    Span span;
    span.offset = offset;
    span.length = length;
    span.line = line;
    span.column = offset - start + 1;
    return span;
}

QString Source::lineText(int line) const
{
    if (line < 1 || line > lineStarts_.size())
        return QString();

    const int start = lineStarts_.at(line - 1);
    const int end = (line < lineStarts_.size()) ? lineStarts_.at(line) : text_.size();

    QString text = text_.mid(start, end - start);
    while (text.endsWith(QLatin1Char('\n')) || text.endsWith(QLatin1Char('\r')))
        text.chop(1);
    return text;
}

} // namespace logo
