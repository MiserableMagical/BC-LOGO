#pragma once

#include <QHash>
#include <QString>

namespace logo {

/// 解析期需要的过程签名：只知道参数个数，用于判断一次调用要读几个表达式。
/// 过程体本身在 AST 的 ProcDefStmt 里。
/// 名字一律传小写（Parser 负责归一化）。
class ProcedureTable {
public:
    void define(const QString &lowerName, int arity) { arities_.insert(lowerName, arity); }
    bool contains(const QString &lowerName) const { return arities_.contains(lowerName); }

    /// 未定义时返回 -1。
    int arityOf(const QString &lowerName) const { return arities_.value(lowerName, -1); }

    void clear() { arities_.clear(); }
    int size() const { return arities_.size(); }

private:
    QHash<QString, int> arities_;
};

} // namespace logo
