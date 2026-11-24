#pragma once

#include <QString>

#include <cmath>
#include <utility>

namespace logo {

/// Logo 数据值。当前支持数字和词；表/数组后续扩展。
class Value {
public:
    enum class Kind {
        Number,
        Word
    };

    Value() = default;

    static Value number(double value)
    {
        Value result;
        result.kind_ = Kind::Number;
        result.number_ = value;
        return result;
    }

    static Value word(QString text)
    {
        Value result;
        result.kind_ = Kind::Word;
        result.word_ = std::move(text);
        return result;
    }

    Kind kind() const { return kind_; }
    bool isNumber() const { return kind_ == Kind::Number; }
    bool isWord() const { return kind_ == Kind::Word; }

    double number() const { return number_; }
    const QString &word() const { return word_; }

    QString toString() const
    {
        if (kind_ == Kind::Number)
            return QString::number(number_, 'g', 15);
        return word_;
    }

    /// `=` 的语义：数字按 epsilon 比较，词大小写不敏感。
    bool equals(const Value &other) const
    {
        if (kind_ != other.kind_)
            return false;
        if (kind_ == Kind::Number)
            return std::abs(number_ - other.number_) < 1e-11;
        return word_.compare(other.word_, Qt::CaseInsensitive) == 0;
    }

private:
    Kind kind_ = Kind::Number;
    double number_ = 0.0;
    QString word_;
};

} // namespace logo
