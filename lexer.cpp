#include "QString"
#include "set"
#include "lexer.h"
#include "QDebug"
using std::set;
extern set<QString> defaultNames;

QChar Lexer::currentChar() const {
    return position_ < input_.size() ? input_[position_] : '\0';
}

bool Lexer::isalnum(QChar ch){
    return ch.isDigit() || ch.isLetter();
}

void Lexer::advance() {
    if (position_ < input_.size()) {
        if (currentChar() == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        position_++;
    }
}

void Lexer::skipWhitespace() {
    while ((currentChar().isSpace()) || currentChar() == ':' || currentChar() == '\'') {
        advance();
    }
}

Token Lexer::parseNumber() {
    QString lexeme;
    int start_line = line_;
    int start_col = column_;
    bool hasDot = false;
    bool invalid = false;

    while ((currentChar().isDigit()) || currentChar() == '.') {
        if (currentChar() == '.') {
            if (hasDot) {
                invalid = true;
            }
            hasDot = true;
        }
        lexeme += currentChar();
        advance();
    }

    // Check for invalid cases
    if (invalid || lexeme.isEmpty() ||
        (lexeme.front() == '.' && lexeme.size() == 1) ||
        lexeme.back() == '.') {
        return Token(TokenType::INVALID, lexeme, 0, start_line, start_col);
    }

    return Token(TokenType::NUMBER, lexeme, lexeme.toDouble(), start_line, start_col);
}

//parse Identifier or keyword, but specially treat "AND" "OR"
Token Lexer::parseIdentifierOrKeyword() {
    QString lexeme;
    int start_line = line_;
    int start_col = column_;

    while (isalnum(currentChar()) || currentChar() == '#' || currentChar() == '_') {
        QChar c = currentChar();
        if(c.isLower()) c = c.toUpper();
        lexeme += c;
        advance();
    }

    TokenType type = defaultNames.count(lexeme) ? TokenType::KEYWORD : TokenType::IDENTIFIER;

    if(lexeme == "AND") {
        lexeme = "&";
        type = TokenType::OPERATOR;
    }

    if(lexeme == "OR") {
        lexeme = "|";
        type = TokenType::OPERATOR;
    }

    return Token(type, lexeme, 0, start_line, start_col);
}

Token Lexer::nextToken() {
    skipWhitespace();
    if (position_ >= input_.size()) {
        return Token(TokenType::END_OF_INPUT, "", 0, line_, column_);
    }

    QChar c = currentChar();
    if (c.isLetter() || c == '#') {
        return parseIdentifierOrKeyword();
    }
    else if (c.isDigit() || c == '.') {
        return parseNumber();
    }
    else {
        int start_line = line_;
        int start_col = column_;
        QString charStr(1, c);
        advance();
        QChar nxt = currentChar();

        switch (c.toLatin1()) {
        case '(': return Token(TokenType::LPAREN, charStr, 0, start_line, start_col);
        case ')': return Token(TokenType::RPAREN, charStr, 0, start_line, start_col);
        case '[': return Token(TokenType::LBRACKET, charStr, 0, start_line, start_col);
        case ']': return Token(TokenType::RBRACKET, charStr, 0, start_line, start_col);
        case ',': return Token(TokenType::COMMA, charStr, 0, start_line, start_col);
        case ';': return Token(TokenType::SEMICOLON, charStr, 0, start_line, start_col);
        case '+': case '-': case '*': case '/': case '=': case '%' :
            return Token(TokenType::OPERATOR, charStr, 0, start_line, start_col);
        case '>': case '<':
            if(nxt == '=')
            {
                advance();
                return Token(TokenType::OPERATOR, charStr + nxt, 0, start_line, start_col);
            }
            return Token(TokenType::OPERATOR, charStr, 0, start_line, start_col);
        default:
            return Token(TokenType::INVALID, charStr, 0, start_line, start_col);
        }
    }
}

void Lexer::Initialize(const QString &input)
{
    position_ = 0;
    line_ = 1;
    column_ = 1;
    input_ = input;
}

// 辅助函数：将TokenType转换为可读字符串
QString Lexer::tokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::KEYWORD: return "KEYWORD";
    case TokenType::IDENTIFIER: return "IDENTIFIER";
    case TokenType::NUMBER: return "NUMBER";
    case TokenType::OPERATOR: return "OPERATOR";
    case TokenType::LPAREN: return "LPAREN";
    case TokenType::RPAREN: return "RPAREN";
    case TokenType::LBRACKET: return "LBRACKET";
    case TokenType::RBRACKET: return "RBRACKET";
    case TokenType::COMMA: return "COMMA";
    case TokenType::SEMICOLON: return "SEMICOLON";
    case TokenType::END_OF_INPUT: return "END_OF_INPUT";
    case TokenType::INVALID: return "INVALID";
    default: return "UNKNOWN";
    }
}
