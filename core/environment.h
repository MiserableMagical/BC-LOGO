#pragma once

#include <QHash>
#include <QString>
#include <QVector>

#include <optional>
#include <utility>

#include "value.h"

namespace logo {

/// 过程调用栈：frames_[0] 是全局帧，最后一个是当前过程帧。
/// 名字大小写不敏感；查找自顶向下（Logo 的动态作用域）。
class Environment {
public:
    Environment() { frames_.append(QHash<QString, Value>()); }

    int depth() const { return frames_.size(); }

    std::optional<Value> lookup(const QString &name) const
    {
        const QString key = name.toLower();
        for (int i = frames_.size() - 1; i >= 0; --i) {
            const auto it = frames_.at(i).constFind(key);
            if (it != frames_.at(i).constEnd())
                return it.value();
        }
        return std::nullopt;
    }

    /// LOCALMAKE：写当前帧。
    void setLocal(const QString &name, const Value &value)
    {
        frames_.last().insert(name.toLower(), value);
    }

    /// MAKE：写最近的同名变量；没有就创建全局变量。
    void setNearestOrGlobal(const QString &name, const Value &value)
    {
        const QString key = name.toLower();
        for (int i = frames_.size() - 1; i >= 0; --i) {
            auto it = frames_[i].find(key);
            if (it != frames_[i].end()) {
                it.value() = value;
                return;
            }
        }
        frames_.first().insert(key, value);
    }

    void pushFrame(QHash<QString, Value> locals)
    {
        frames_.append(std::move(locals));
    }

    void popFrame()
    {
        if (frames_.size() > 1)
            frames_.removeLast();
    }

private:
    QVector<QHash<QString, Value>> frames_;
};

} // namespace logo
