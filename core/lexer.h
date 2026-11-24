#pragma once

#include <QVector>

#include "token.h"

namespace logo {

class Source;
class Diagnostics;

/// 词法器：只负责把字符切成 Token，不认识关键字、不做语义判断。
///
/// 词法约定：
///  - `:name` 产生 Variable token，text 是去掉冒号的名字；
///  - `"word` 产生 quoted Word（词字面量）；
///  - 裸词产生普通 Word，交给 Parser 判断是命令还是过程；
///  - `#` 可以出现在词的开头或中间（用于颜色字面量）；
///  - `;` 到行尾是注释；
///  - 换行单独产生 Newline token。
///
/// 与旧 Lexer 的差异（有意为之）：
///  - `:` 不再当空白：变量必须写 `:X`，对应原版 Logo 的用法；
///  - `'` 不再特殊处理，会成为非法字符；
///  - 保留原始大小写，不再在词法阶段折叠；
///  - 支持 `.5`、`5.`、`1e3`、`1.5E-2`；
///  - 非法字符产生 Invalid 并继续扫描。
class Lexer {
public:
    explicit Lexer(const Source &source, Diagnostics *diagnostics = nullptr);

    Token next();
    QVector<Token> tokenizeAll();

private:
    QChar current() const;
    bool atEnd() const;
    void advance();
    void skipTrivia();

    bool startsExponent() const;

    Token make(TokenKind kind, int startOffset, int startLine, int startColumn) const;
    Token invalidToken(int startOffset, int startLine, int startColumn,
                       const QString &code, const QString &message);

    Token scanWord();
    Token scanVariable();
    Token scanQuotedWord();
    Token scanNumber();
    Token scanOperator();
    Token scanNewline();

    const Source &source_;
    Diagnostics *diagnostics_ = nullptr;
    int pos_ = 0;
    int line_ = 1;
    int column_ = 1;
};

} // namespace logo
