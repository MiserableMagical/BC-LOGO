#include "parser.h"

#include "diagnostic.h"
#include "diagnostic_codes.h"
#include "lexer.h"
#include "source.h"

#include <utility>

namespace logo {

namespace {

bool isControlWord(const QString &lower)
{
    return lower == QLatin1String("to") || lower == QLatin1String("end")
        || lower == QLatin1String("repeat") || lower == QLatin1String("if")
        || lower == QLatin1String("ifelse") || lower == QLatin1String("while")
        || lower == QLatin1String("make") || lower == QLatin1String("localmake")
        || lower == QLatin1String("stop") || lower == QLatin1String("output")
        || lower == QLatin1String("op");
}

bool isEndWord(const Token &token)
{
    return token.kind == TokenKind::Word
        && token.text.compare(QStringLiteral("end"), Qt::CaseInsensitive) == 0;
}

QString prefixOperationName(const QString &lower)
{
    if (lower == QLatin1String("sin") || lower == QLatin1String("cos")
        || lower == QLatin1String("random")) {
        return lower;
    }
    return QString();
}

/// 返回 0 表示不是中缀运算符；数值越大结合越紧。
int infixPrecedence(const Token &token)
{
    if (token.kind == TokenKind::Operator) {
        const QString &op = token.text;
        if (op == QLatin1String("="))
            return 10;
        if (op == QLatin1String("<") || op == QLatin1String(">")
            || op == QLatin1String("<=") || op == QLatin1String(">=")) {
            return 20;
        }
        if (op == QLatin1String("|"))
            return 30;
        if (op == QLatin1String("&"))
            return 40;
        if (op == QLatin1String("+") || op == QLatin1String("-"))
            return 50;
        if (op == QLatin1String("*") || op == QLatin1String("/") || op == QLatin1String("%"))
            return 60;
        return 0;
    }

    if (token.kind == TokenKind::Word) {
        const QString lower = token.text.toLower();
        if (lower == QLatin1String("or"))
            return 30;
        if (lower == QLatin1String("and"))
            return 40;
    }
    return 0;
}

QString infixOperatorName(const Token &token)
{
    if (token.kind == TokenKind::Word) {
        const QString lower = token.text.toLower();
        if (lower == QLatin1String("and"))
            return QStringLiteral("&");
        if (lower == QLatin1String("or"))
            return QStringLiteral("|");
        return lower;
    }
    return token.text;
}

bool isUnaryOperator(const Token &token)
{
    return token.kind == TokenKind::Operator
        && (token.text == QLatin1String("+") || token.text == QLatin1String("-"));
}

bool canStartExpression(const Token &token)
{
    switch (token.kind) {
    case TokenKind::Number:
    case TokenKind::LParen:
    case TokenKind::Word:
    case TokenKind::Variable:
        return true;
    case TokenKind::Operator:
        return token.text == QLatin1String("+") || token.text == QLatin1String("-");
    default:
        return false;
    }
}

} // namespace

Parser::Parser(const Source &source, Diagnostics &diagnostics)
    : source_(source)
    , diagnostics_(diagnostics)
    , procedures_(&ownedProcedures_)
{
    Lexer lexer(source_, &diagnostics_);
    tokens_ = lexer.tokenizeAll();
}

Parser::Parser(const Source &source, Diagnostics &diagnostics, ProcedureTable &procedures)
    : source_(source)
    , diagnostics_(diagnostics)
    , procedures_(&procedures)
{
    Lexer lexer(source_, &diagnostics_);
    tokens_ = lexer.tokenizeAll();
}

// ---------------------------------------------------------------------------
// token 流
// ---------------------------------------------------------------------------

const Token &Parser::peek(int ahead) const
{
    if (tokens_.isEmpty()) {
        static const Token eof;
        return eof;
    }
    const int last = static_cast<int>(tokens_.size()) - 1;
    const int index = qBound(0, pos_ + ahead, last);
    return tokens_.at(index);
}

const Token &Parser::advance()
{
    const Token &token = peek();
    if (pos_ < static_cast<int>(tokens_.size()) - 1)
        ++pos_;
    return token;
}

bool Parser::check(TokenKind kind) const
{
    return peek().kind == kind;
}

bool Parser::match(TokenKind kind)
{
    if (!check(kind))
        return false;
    advance();
    return true;
}

void Parser::skipNewlines()
{
    while (check(TokenKind::Newline))
        advance();
}

Span Parser::spanFrom(const Token &first) const
{
    Span span = first.span;
    if (pos_ > 0) {
        const int end = tokens_.at(pos_ - 1).span.endOffset();
        if (end > first.span.offset)
            span.length = end - first.span.offset;
    }
    return span;
}

void Parser::error(const QString &code, const QString &message, const Span &span)
{
    diagnostics_.error(code, message, span);
}

void Parser::synchronize()
{
    while (!check(TokenKind::Eof) && !check(TokenKind::Newline)
           && !check(TokenKind::RBracket)) {
        advance();
    }
    if (check(TokenKind::Newline))
        advance();
}

// ---------------------------------------------------------------------------
// 顶层
// ---------------------------------------------------------------------------

Program Parser::parseProgram()
{
    Program program;
    program.span = source_.span(0, source_.length());

    while (!check(TokenKind::Eof)) {
        skipNewlines();
        if (check(TokenKind::Eof))
            break;

        if (check(TokenKind::Invalid)) {
            advance();
            continue;
        }

        const int before = pos_;
        StmtPtr stmt = parseInstruction();
        if (stmt)
            program.statements.push_back(std::move(stmt));

        // 防御：任何路径都必须前进，避免死循环。
        if (pos_ == before && !check(TokenKind::Eof))
            advance();
    }
    return program;
}

StmtPtr Parser::parseInstruction()
{
    skipNewlines();
    if (check(TokenKind::Eof))
        return nullptr;

    if (check(TokenKind::Invalid)) {
        advance();
        return nullptr;
    }

    const Token &first = peek();

    if (first.kind == TokenKind::RBracket) {
        error(diag::kUnexpectedToken, QStringLiteral("unexpected ']'"), first.span);
        advance();
        synchronize();
        return nullptr;
    }
    if (first.kind == TokenKind::Comma || first.kind == TokenKind::RParen) {
        error(diag::kUnexpectedToken,
              QStringLiteral("unexpected '%1'").arg(first.text), first.span);
        advance();
        synchronize();
        return nullptr;
    }

    if (first.kind == TokenKind::Word) {
        // 引号词永远是字面量，不能被当成命令。
        if (first.quoted)
            return parseExpressionStatement(first);

        const QString lower = first.text.toLower();

        if (lower == QLatin1String("to")) {
            if (definitionDepth_ > 0) {
                error(diag::kUnexpectedToken,
                      QStringLiteral("TO inside a procedure definition"), first.span);
                advance();
                synchronize();
                return nullptr;
            }
            return parseDefinition(first);
        }

        if (isEndWord(first)) {
            error(diag::kUnexpectedEnd, QStringLiteral("END without TO"), first.span);
            advance();
            synchronize();
            return nullptr;
        }

        if (lower == QLatin1String("output") || lower == QLatin1String("op")) {
            const bool insideProcedure = definitionDepth_ > 0;
            advance();

            const int before = diagnostics_.count();
            ExprPtr value = parseExpression(0);
            if (!value) {
                if (diagnostics_.count() == before) {
                    error(diag::kExpectedExpression,
                          QStringLiteral("OUTPUT needs an expression"), peek().span);
                }
                synchronize();
                return nullptr;
            }

            if (!insideProcedure) {
                error(diag::kOutputOutsideProcedure,
                      QStringLiteral("OUTPUT can only be used inside a procedure"),
                      first.span);
            }
            return std::make_unique<OutputStmt>(std::move(value), spanFrom(first));
        }

        if (lower == QLatin1String("repeat"))
            return parseRepeat(first);
        if (lower == QLatin1String("if"))
            return parseIf(first);
        if (lower == QLatin1String("ifelse"))
            return parseIfElse(first);
        if (lower == QLatin1String("while"))
            return parseWhile(first);
        if (lower == QLatin1String("make"))
            return parseMake(first, false);
        if (lower == QLatin1String("localmake"))
            return parseMake(first, true);
        if (lower == QLatin1String("stop")) {
            advance();
            return std::make_unique<StopStmt>(first.span);
        }

        if (const Builtin *builtin = builtins_.find(lower)) {
            advance();
            return parseCommand(first, *builtin);
        }

        if (procedures_->contains(lower)) {
            const int arity = procedures_->arityOf(lower);
            advance();
            return parseProcedureCall(first, arity);
        }

        if (!prefixOperationName(lower).isEmpty())
            return parseExpressionStatement(first);

        error(diag::kUnknownCommand,
              QStringLiteral("unknown command '%1'").arg(first.text), first.span);
        advance();
        synchronize();
        return nullptr;
    }

    if (canStartExpression(first))
        return parseExpressionStatement(first);

    error(diag::kUnexpectedToken,
          QStringLiteral("unexpected token '%1'").arg(first.text), first.span);
    advance();
    synchronize();
    return nullptr;
}

StmtPtr Parser::parseCommand(const Token &nameToken, const Builtin &builtin)
{
    std::vector<ExprPtr> args;
    if (builtin.arity > 0)
        args.reserve(static_cast<std::size_t>(builtin.arity));

    for (int i = 0; i < builtin.arity; ++i) {
        const int before = diagnostics_.count();
        ExprPtr expr = parseExpression(0);
        if (!expr) {
            if (diagnostics_.count() == before) {
                error(diag::kExpectedExpression,
                      QStringLiteral("'%1' needs %2 input(s)")
                          .arg(builtin.canonical)
                          .arg(builtin.arity),
                      peek().span);
            }
            synchronize();
            return nullptr;
        }
        args.push_back(std::move(expr));
    }

    return std::make_unique<CommandStmt>(builtin.canonical, std::move(args),
                                        spanFrom(nameToken));
}

StmtPtr Parser::parseProcedureCall(const Token &nameToken, int arity)
{
    std::vector<ExprPtr> args;
    if (arity > 0)
        args.reserve(static_cast<std::size_t>(arity));

    for (int i = 0; i < arity; ++i) {
        const int before = diagnostics_.count();
        ExprPtr arg = parseExpression(0);
        if (!arg) {
            if (diagnostics_.count() == before) {
                error(diag::kExpectedExpression,
                      QStringLiteral("'%1' needs %2 input(s)")
                          .arg(nameToken.text)
                          .arg(arity),
                      peek().span);
            }
            synchronize();
            return nullptr;
        }
        args.push_back(std::move(arg));
    }

    return std::make_unique<CommandStmt>(nameToken.text.toLower(), std::move(args),
                                        spanFrom(nameToken));
}

StmtPtr Parser::parseDefinition(const Token &toToken)
{
    advance(); // TO

    if (!check(TokenKind::Word)) {
        error(diag::kExpectedWord,
              QStringLiteral("TO needs a procedure name"), peek().span);
        synchronize();
        return nullptr;
    }

    const Token nameToken = advance();
    const QString name = nameToken.text.toLower();

    if (builtins_.contains(name) || isControlWord(name) || !prefixOperationName(name).isEmpty()) {
        error(diag::kReservedName,
              QStringLiteral("'%1' is already in use").arg(nameToken.text), nameToken.span);
        // 整段吞掉，避免过程体被当成顶层语句执行。
        while (!check(TokenKind::Eof)) {
            if (isEndWord(peek())) {
                advance();
                break;
            }
            advance();
        }
        return nullptr;
    }

    std::vector<QString> params;
    while (check(TokenKind::Variable))
        params.push_back(advance().text);

    if (isEndWord(peek())) {
        // 空过程体：让下面的循环消费 END。
    } else if (check(TokenKind::Word)) {
        error(diag::kExpectedParameter,
              QStringLiteral("expected a :parameter, got '%1'").arg(peek().text),
              peek().span);
        synchronize();
    } else if (check(TokenKind::Newline)) {
        skipNewlines();
    } else if (!check(TokenKind::Eof)) {
        error(diag::kUnexpectedToken,
              QStringLiteral("unexpected token in TO header: '%1'").arg(peek().text),
              peek().span);
        synchronize();
    }

    // 先登记 arity，过程体里的递归调用才认得出自己。
    procedures_->define(name, static_cast<int>(params.size()));

    ++definitionDepth_;
    StmtList body;
    bool sawEnd = false;
    while (!check(TokenKind::Eof)) {
        skipNewlines();
        if (check(TokenKind::Eof))
            break;
        if (isEndWord(peek())) {
            advance();
            sawEnd = true;
            break;
        }

        const int before = pos_;
        StmtPtr stmt = parseInstruction();
        if (stmt)
            body.push_back(std::move(stmt));
        if (pos_ == before && !check(TokenKind::Eof))
            advance();
    }
    --definitionDepth_;

    if (!sawEnd) {
        error(diag::kMissingEnd,
              QStringLiteral("TO '%1' has no END").arg(nameToken.text), toToken.span);
    }

    return std::make_unique<ProcDefStmt>(name, std::move(params), std::move(body),
                                        spanFrom(toToken));
}

StmtPtr Parser::parseRepeat(const Token &keyword)
{
    advance();
    const int before = diagnostics_.count();
    ExprPtr times = parseExpression(0);
    if (!times) {
        if (diagnostics_.count() == before) {
            error(diag::kExpectedExpression,
                  QStringLiteral("REPEAT needs a count expression"), peek().span);
        }
        synchronize();
        return nullptr;
    }

    skipNewlines();
    StmtList body = parseBlock();
    return std::make_unique<RepeatStmt>(std::move(times), std::move(body),
                                       spanFrom(keyword));
}

StmtPtr Parser::parseIf(const Token &keyword)
{
    advance();
    const int before = diagnostics_.count();
    ExprPtr cond = parseExpression(0);
    if (!cond) {
        if (diagnostics_.count() == before) {
            error(diag::kExpectedExpression,
                  QStringLiteral("IF needs a condition"), peek().span);
        }
        synchronize();
        return nullptr;
    }

    skipNewlines();
    StmtList thenBody = parseBlock();
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBody),
                                   spanFrom(keyword));
}

StmtPtr Parser::parseIfElse(const Token &keyword)
{
    advance();
    const int before = diagnostics_.count();
    ExprPtr cond = parseExpression(0);
    if (!cond) {
        if (diagnostics_.count() == before) {
            error(diag::kExpectedExpression,
                  QStringLiteral("IFELSE needs a condition"), peek().span);
        }
        synchronize();
        return nullptr;
    }

    skipNewlines();
    StmtList thenBody = parseBlock();
    skipNewlines();
    StmtList elseBody = parseBlock();
    return std::make_unique<IfElseStmt>(std::move(cond), std::move(thenBody),
                                       std::move(elseBody), spanFrom(keyword));
}

StmtPtr Parser::parseWhile(const Token &keyword)
{
    advance();
    const int before = diagnostics_.count();
    ExprPtr cond = parseExpression(0);
    if (!cond) {
        if (diagnostics_.count() == before) {
            error(diag::kExpectedExpression,
                  QStringLiteral("WHILE needs a condition"), peek().span);
        }
        synchronize();
        return nullptr;
    }

    skipNewlines();
    StmtList body = parseBlock();
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body),
                                      spanFrom(keyword));
}

StmtPtr Parser::parseMake(const Token &keyword, bool local)
{
    advance();
    const QString keywordName = local ? QStringLiteral("LOCALMAKE") : QStringLiteral("MAKE");

    const int beforeName = diagnostics_.count();
    ExprPtr name = parseExpression(0);
    if (!name) {
        if (diagnostics_.count() == beforeName) {
            error(diag::kExpectedExpression,
                  QStringLiteral("%1 needs a variable name expression").arg(keywordName),
                  peek().span);
        }
        synchronize();
        return nullptr;
    }

    const int beforeValue = diagnostics_.count();
    ExprPtr value = parseExpression(0);
    if (!value) {
        if (diagnostics_.count() == beforeValue) {
            error(diag::kExpectedExpression,
                  QStringLiteral("%1 needs a value").arg(keywordName), peek().span);
        }
        synchronize();
        return nullptr;
    }

    return std::make_unique<MakeStmt>(std::move(name), std::move(value), local,
                                     spanFrom(keyword));
}

StmtPtr Parser::parseExpressionStatement(const Token &first)
{
    const int before = diagnostics_.count();
    ExprPtr expr = parseExpression(0);
    if (!expr) {
        if (diagnostics_.count() == before)
            error(diag::kExpectedExpression, QStringLiteral("expected an expression"), first.span);
        synchronize();
        return nullptr;
    }
    return std::make_unique<ExprStmt>(std::move(expr), spanFrom(first));
}

StmtList Parser::parseBlock()
{
    StmtList body;

    if (!check(TokenKind::LBracket)) {
        error(diag::kExpectedBlock, QStringLiteral("expected '['"), peek().span);
        return body;
    }

    const Token open = advance();
    skipNewlines();

    while (!check(TokenKind::RBracket) && !check(TokenKind::Eof)) {
        const int before = pos_;
        StmtPtr stmt = parseInstruction();
        if (stmt)
            body.push_back(std::move(stmt));

        if (pos_ == before && !check(TokenKind::RBracket) && !check(TokenKind::Eof))
            advance();
        skipNewlines();
    }

    if (check(TokenKind::RBracket)) {
        advance();
    } else {
        error(diag::kUnterminatedBlock, QStringLiteral("unterminated block"), open.span);
    }
    return body;
}

// ---------------------------------------------------------------------------
// 表达式
// ---------------------------------------------------------------------------

ExprPtr Parser::parseExpression(int minBindingPower)
{
    ExprPtr lhs = parseAtom();
    if (!lhs)
        return nullptr;

    for (;;) {
        // 括号内允许运算符出现在续行开头，例如 (1\n+ 2)。
        if (parenDepth_ > 0)
            skipNewlines();

        const Token &token = peek();
        const int precedence = infixPrecedence(token);
        if (precedence <= minBindingPower)
            break;

        const Token opToken = advance();
        const QString op = infixOperatorName(opToken);

        // 只有括号内才允许运算符后续行，避免把下一行的命令吞成操作数。
        if (parenDepth_ > 0)
            skipNewlines();

        const int before = diagnostics_.count();
        ExprPtr rhs = parseExpression(precedence);
        if (!rhs) {
            if (diagnostics_.count() == before) {
                error(diag::kExpectedExpression,
                      QStringLiteral("expected an expression after '%1'").arg(opToken.text),
                      peek().span);
            }
            return nullptr;
        }

        Span span = lhs->span;
        const int end = rhs->span.endOffset();
        span.length = (end > span.offset) ? end - span.offset : 0;
        lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs), span);
    }
    return lhs;
}

ExprPtr Parser::parseAtom()
{
    const Token &token = peek();

    if (token.kind == TokenKind::Number) {
        const Token number = advance();
        return std::make_unique<NumberExpr>(number.number, number.span);
    }

    if (token.kind == TokenKind::LParen)
        return parseParenthesized();

    if (isUnaryOperator(token)) {
        const Token op = advance();
        if (parenDepth_ > 0)
            skipNewlines();

        const int before = diagnostics_.count();
        ExprPtr operand = parseExpression(70);
        if (!operand) {
            if (diagnostics_.count() == before) {
                error(diag::kExpectedExpression,
                      QStringLiteral("expected an expression after '%1'").arg(op.text),
                      peek().span);
            }
            return nullptr;
        }

        const int end = operand->span.endOffset();
        Span span = op.span;
        span.length = (end > span.offset) ? end - span.offset : 0;
        return std::make_unique<UnaryExpr>(op.text, std::move(operand), span);
    }

    if (token.kind == TokenKind::Variable) {
        const Token variable = advance();
        return std::make_unique<WordExpr>(variable.text, false, variable.span);
    }

    if (token.kind == TokenKind::Word) {
        if (token.quoted) {
            const Token word = advance();
            return std::make_unique<WordExpr>(word.text, true, word.span);
        }

        const QString lower = token.text.toLower();
        if (lower == QLatin1String("and") || lower == QLatin1String("or")) {
            error(diag::kExpectedExpression,
                  QStringLiteral("expected an expression before '%1'").arg(token.text),
                  token.span);
            return nullptr;
        }

        const Token name = advance();

        const QString operation = prefixOperationName(lower);
        if (!operation.isEmpty())
            return parseCallExpression(name, operation, 1);

        if (procedures_->contains(lower))
            return parseCallExpression(name, lower, procedures_->arityOf(lower));

        if (builtins_.contains(lower) || isControlWord(lower)) {
            error(diag::kCommandAsValue,
                  QStringLiteral("'%1' is a command and cannot be used as a value")
                      .arg(name.text),
                  name.span);
            return nullptr;
        }

        error(diag::kUnknownCommand,
              QStringLiteral("unknown procedure '%1'").arg(name.text), name.span);
        return nullptr;
    }

    return nullptr;
}

ExprPtr Parser::parseCallExpression(const Token &nameToken, const QString &callee, int arity)
{
    std::vector<ExprPtr> args;
    if (arity > 0)
        args.reserve(static_cast<std::size_t>(arity));

    for (int i = 0; i < arity; ++i) {
        if (parenDepth_ > 0)
            skipNewlines();

        const int before = diagnostics_.count();
        ExprPtr arg = parseExpression(70);
        if (!arg) {
            if (diagnostics_.count() == before) {
                error(diag::kExpectedExpression,
                      QStringLiteral("'%1' needs %2 input(s)").arg(callee).arg(arity),
                      peek().span);
            }
            return nullptr;
        }
        args.push_back(std::move(arg));
    }

    Span span = nameToken.span;
    if (!args.empty()) {
        const int end = args.back()->span.endOffset();
        span.length = (end > span.offset) ? end - span.offset : 0;
    }
    return std::make_unique<CallExpr>(callee, std::move(args), span);
}

ExprPtr Parser::parseParenthesized()
{
    const Token open = advance(); // (
    ++parenDepth_;
    skipNewlines();

    const int before = diagnostics_.count();
    ExprPtr inner = parseExpression(0);
    skipNewlines();
    --parenDepth_;

    if (!check(TokenKind::RParen)) {
        error(diag::kExpectedParen, QStringLiteral("expected ')'"), peek().span);
    } else {
        const Token close = advance();
        if (inner) {
            Span span = open.span;
            span.length = close.span.endOffset() - open.span.offset;
            inner->span = span;
        }
    }

    if (!inner && diagnostics_.count() == before) {
        error(diag::kExpectedExpression,
              QStringLiteral("expected an expression inside '(...)'"), open.span);
    }
    return inner;
}

} // namespace logo
