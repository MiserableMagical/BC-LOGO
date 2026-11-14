#include "lexer.h"

#include "diagnostic.h"
#include "diagnostic_codes.h"
#include "source.h"

namespace logo {

Lexer::Lexer(const Source &source, Diagnostics *diagnostics)
    : source_(source)
    , diagnostics_(diagnostics)
{
}

bool Lexer::atEnd() const
{
    return pos_ >= source_.text().size();
}

QChar Lexer::current() const
{
    return atEnd() ? QChar() : source_.text().at(pos_);
}

void Lexer::advance()
{
    if (atEnd())
        return;

    const QChar c = source_.text().at(pos_++);
    if (c == QLatin1Char('\n')) {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
}

void Lexer::skipTrivia()
{
    for (;;) {
        if (atEnd())
            return;

        const QChar c = current();
        if (c == QLatin1Char('\n') || c == QLatin1Char('\r'))
            return;

        // 旧 Lexer 把 `:` 和 `'` 当作空白跳过；保持兼容。
        if (c.isSpace() || c == QLatin1Char(':') || c == QLatin1Char('\'')) {
            advance();
            continue;
        }

        if (c == QLatin1Char(';')) {
            while (!atEnd() && current() != QLatin1Char('\n') && current() != QLatin1Char('\r'))
                advance();
            continue;
        }

        return;
    }
}

Token Lexer::make(TokenKind kind, int startOffset, int startLine, int startColumn) const
{
    Token token;
    token.kind = kind;
    token.span.offset = startOffset;
    token.span.length = pos_ - startOffset;
    token.span.line = startLine;
    token.span.column = startColumn;
    token.text = source_.text().mid(startOffset, token.span.length);
    return token;
}

Token Lexer::invalidToken(int startOffset, int startLine, int startColumn,
                          const QString &code, const QString &message)
{
    Token token = make(TokenKind::Invalid, startOffset, startLine, startColumn);
    if (diagnostics_)
        diagnostics_->error(code, message, token.span);
    return token;
}

bool Lexer::startsExponent() const
{
    const QString &text = source_.text();
    int index = pos_;
    if (index >= text.size())
        return false;

    const QChar e = text.at(index);
    if (e != QLatin1Char('e') && e != QLatin1Char('E'))
        return false;

    ++index;
    if (index < text.size()
        && (text.at(index) == QLatin1Char('+') || text.at(index) == QLatin1Char('-'))) {
        ++index;
    }
    return index < text.size() && text.at(index).isDigit();
}

Token Lexer::scanNumber()
{
    const int startOffset = pos_;
    const int startLine = line_;
    const int startColumn = column_;
    bool hasDigit = false;

    while (!atEnd() && current().isDigit()) {
        hasDigit = true;
        advance();
    }

    if (!atEnd() && current() == QLatin1Char('.')) {
        advance();
        while (!atEnd() && current().isDigit()) {
            hasDigit = true;
            advance();
        }
    }

    // 只有确认 e/E 后面确实跟着数字时才吃掉指数，避免把 `1e` 这类输入吞坏。
    if (hasDigit && startsExponent()) {
        advance(); // e / E
        if (!atEnd() && (current() == QLatin1Char('+') || current() == QLatin1Char('-')))
            advance();
        while (!atEnd() && current().isDigit())
            advance();
    }

    Token token = make(TokenKind::Number, startOffset, startLine, startColumn);

    if (!hasDigit) {
        token.kind = TokenKind::Invalid;
        if (diagnostics_) {
            diagnostics_->error(diag::kMalformedNumber,
                                QStringLiteral("malformed number '%1'").arg(token.text),
                                token.span);
        }
        return token;
    }

    bool ok = false;
    const double value = token.text.toDouble(&ok);
    if (!ok) {
        token.kind = TokenKind::Invalid;
        if (diagnostics_) {
            diagnostics_->error(diag::kMalformedNumber,
                                QStringLiteral("malformed number '%1'").arg(token.text),
                                token.span);
        }
        return token;
    }

    token.number = value;
    return token;
}

Token Lexer::scanWord()
{
    const int startOffset = pos_;
    const int startLine = line_;
    const int startColumn = column_;

    advance(); // 首字符由 next() 保证合法

    while (!atEnd()) {
        const QChar c = current();
        if (c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('#'))
            advance();
        else
            break;
    }

    return make(TokenKind::Word, startOffset, startLine, startColumn);
}

Token Lexer::scanQuotedWord()
{
    const int startOffset = pos_;
    const int startLine = line_;
    const int startColumn = column_;

    advance(); // 开头的双引号

    while (!atEnd()) {
        const QChar c = current();
        if (c.isSpace() || c == QLatin1Char('[') || c == QLatin1Char(']')
            || c == QLatin1Char('(') || c == QLatin1Char(')')
            || c == QLatin1Char(',') || c == QLatin1Char(';')) {
            break;
        }
        advance();
    }

    Token token = make(TokenKind::Word, startOffset, startLine, startColumn);
    token.quoted = true;
    token.text = source_.text().mid(startOffset + 1, token.span.length - 1);
    return token;
}

Token Lexer::scanNewline()
{
    const int startOffset = pos_;
    const int startLine = line_;
    const int startColumn = column_;

    if (current() == QLatin1Char('\r')) {
        advance();
        if (!atEnd() && current() == QLatin1Char('\n')) {
            advance();
        } else {
            // 单独的 CR：advance() 不会自己换行
            ++line_;
            column_ = 1;
        }
    } else {
        advance();
    }

    return make(TokenKind::Newline, startOffset, startLine, startColumn);
}

Token Lexer::scanOperator()
{
    const int startOffset = pos_;
    const int startLine = line_;
    const int startColumn = column_;
    const QChar c = current();

    if (c == QLatin1Char('<') || c == QLatin1Char('>')) {
        advance();
        if (!atEnd() && current() == QLatin1Char('='))
            advance();
        return make(TokenKind::Operator, startOffset, startLine, startColumn);
    }

    if (c == QLatin1Char('+') || c == QLatin1Char('-') || c == QLatin1Char('*')
        || c == QLatin1Char('/') || c == QLatin1Char('%') || c == QLatin1Char('=')
        || c == QLatin1Char('&') || c == QLatin1Char('|')) {
        advance();
        return make(TokenKind::Operator, startOffset, startLine, startColumn);
    }

    if (c == QLatin1Char('(')) {
        advance();
        return make(TokenKind::LParen, startOffset, startLine, startColumn);
    }
    if (c == QLatin1Char(')')) {
        advance();
        return make(TokenKind::RParen, startOffset, startLine, startColumn);
    }
    if (c == QLatin1Char('[')) {
        advance();
        return make(TokenKind::LBracket, startOffset, startLine, startColumn);
    }
    if (c == QLatin1Char(']')) {
        advance();
        return make(TokenKind::RBracket, startOffset, startLine, startColumn);
    }
    if (c == QLatin1Char(',')) {
        advance();
        return make(TokenKind::Comma, startOffset, startLine, startColumn);
    }

    advance();
    return invalidToken(startOffset, startLine, startColumn, diag::kInvalidCharacter,
                        QStringLiteral("unexpected character '%1'").arg(c));
}

Token Lexer::next()
{
    skipTrivia();

    if (atEnd())
        return make(TokenKind::Eof, pos_, line_, column_);

    const QChar c = current();
    if (c == QLatin1Char('\n') || c == QLatin1Char('\r'))
        return scanNewline();
    if (c == QLatin1Char('"'))
        return scanQuotedWord();
    if (c.isDigit() || c == QLatin1Char('.'))
        return scanNumber();
    if (c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('#'))
        return scanWord();
    return scanOperator();
}

QVector<Token> Lexer::tokenizeAll()
{
    QVector<Token> tokens;
    for (;;) {
        const Token token = next();
        tokens.append(token);
        if (token.kind == TokenKind::Eof)
            break;
    }
    return tokens;
}

} // namespace logo
