#include "builtins.h"

namespace logo {

BuiltinTable::BuiltinTable()
{
    add(QStringLiteral("fd"), QStringLiteral("fd"), 1);
    add(QStringLiteral("forward"), QStringLiteral("fd"), 1);

    add(QStringLiteral("bk"), QStringLiteral("bk"), 1);
    add(QStringLiteral("back"), QStringLiteral("bk"), 1);
    add(QStringLiteral("backward"), QStringLiteral("bk"), 1);

    add(QStringLiteral("lt"), QStringLiteral("lt"), 1);
    add(QStringLiteral("left"), QStringLiteral("lt"), 1);

    add(QStringLiteral("rt"), QStringLiteral("rt"), 1);
    add(QStringLiteral("right"), QStringLiteral("rt"), 1);

    add(QStringLiteral("cs"), QStringLiteral("cs"), 0);

    add(QStringLiteral("st"), QStringLiteral("st"), 0);
    add(QStringLiteral("showturtle"), QStringLiteral("st"), 0);

    add(QStringLiteral("ht"), QStringLiteral("ht"), 0);
    add(QStringLiteral("hideturtle"), QStringLiteral("ht"), 0);

    add(QStringLiteral("pu"), QStringLiteral("pu"), 0);
    add(QStringLiteral("penup"), QStringLiteral("pu"), 0);

    add(QStringLiteral("pd"), QStringLiteral("pd"), 0);
    add(QStringLiteral("pendown"), QStringLiteral("pd"), 0);

    add(QStringLiteral("home"), QStringLiteral("home"), 0);
    add(QStringLiteral("clean"), QStringLiteral("clean"), 0);

    add(QStringLiteral("setw"), QStringLiteral("setw"), 1);
    add(QStringLiteral("setwidth"), QStringLiteral("setw"), 1);

    add(QStringLiteral("setpc"), QStringLiteral("setpc"), 1);
    add(QStringLiteral("setbg"), QStringLiteral("setbg"), 1);

    add(QStringLiteral("setpcdec"), QStringLiteral("setpcdec"), 3);

    add(QStringLiteral("setx"), QStringLiteral("setx"), 1);
    add(QStringLiteral("sety"), QStringLiteral("sety"), 1);
    add(QStringLiteral("setxy"), QStringLiteral("setxy"), 2);

    add(QStringLiteral("wait"), QStringLiteral("wait"), 1);
    add(QStringLiteral("print"), QStringLiteral("print"), 1);
}

void BuiltinTable::add(const QString &alias, const QString &canonical, int arity)
{
    byName_.insert(alias, Builtin{canonical, arity});
}

const Builtin *BuiltinTable::find(const QString &lowerName) const
{
    const auto it = byName_.constFind(lowerName);
    return it == byName_.constEnd() ? nullptr : &it.value();
}

bool BuiltinTable::contains(const QString &lowerName) const
{
    return byName_.contains(lowerName);
}

QStringList BuiltinTable::aliases() const
{
    return byName_.keys();
}

QStringList logoVocabulary()
{
    static const QStringList words = [] {
        BuiltinTable table;
        QStringList result = table.aliases();
        result << QStringLiteral("to") << QStringLiteral("end")
               << QStringLiteral("repeat") << QStringLiteral("if")
               << QStringLiteral("ifelse") << QStringLiteral("while")
               << QStringLiteral("make") << QStringLiteral("localmake")
               << QStringLiteral("stop") << QStringLiteral("output")
               << QStringLiteral("op") << QStringLiteral("sin")
               << QStringLiteral("cos") << QStringLiteral("random")
               << QStringLiteral("and") << QStringLiteral("or");
        result.removeDuplicates();
        result.sort();
        return result;
    }();
    return words;
}

} // namespace logo
