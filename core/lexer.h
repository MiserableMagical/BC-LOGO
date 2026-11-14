#pragma once

#include <QVector>

#include "token.h"

namespace logo {

class Source;
class Diagnostics;

/// 词法器：只负责把字符切成 Token，不认识关键字、不做语义判断。
///
/// 兼容性约定（对应旧 Lexer 的行为）：
///  - `:` 和 `'` 在 token 之间被当作空白（旧实现如此）；
///  - `#` 可以出现在词的开头或中间（用于颜色字面量）；
///  - `;` 到行尾是注释；
///  - 换行单独产生 Newline token。
///
/// 与旧 Lexer 的差异（有意为之，供阶段 2 使用）：
///  - 保留原始大小写，不再在词法阶段折叠；
///  - 所有标识符都是 Word，关键字表交给 Parser；
///  - 支持 `.5`、`5.`、`1e3`、`1.5E-2`；
///  - `"word` 产生 quoted Word；非法字符产生 Invalid 并继续扫描。
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
