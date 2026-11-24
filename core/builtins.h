#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

namespace logo {

/// 普通命令的元数据。所有参数都是表达式（原版 Logo 语义），
/// 词字面量通过 `"word` 在表达式里表达。
struct Builtin {
    QString canonical; // 别名归一到规范名，例如 FORWARD -> fd
    int arity = 0;
};

/// 普通命令表。控制形式（REPEAT/IF/OUTPUT/…）在 Parser 里单独处理。
class BuiltinTable {
public:
    BuiltinTable();

    const Builtin *find(const QString &lowerName) const;
    bool contains(const QString &lowerName) const;

    /// 所有别名（小写），用于高亮/补全。
    QStringList aliases() const;

private:
    void add(const QString &alias, const QString &canonical, int arity);

    QHash<QString, Builtin> byName_;
};

/// 命令别名 + 控制词 + 运算词，供语法高亮等使用。
QStringList logoVocabulary();

} // namespace logo
