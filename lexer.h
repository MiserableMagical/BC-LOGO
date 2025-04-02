#ifndef LEXER_H
#define LEXER_H

#endif // LEXER_H

#include "QString"

enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    LPAREN,     // (
    RPAREN,     // )
    LBRACKET,   // [
    RBRACKET,   // ]
    COMMA,
    SEMICOLON,
    END_OF_INPUT,
    INVALID
};

//set<QString> defaultNames = {"FD","FORWARD","BK","BACKWARD","LT","LEFT","RT","RIGHT",
//                             "CS","ST","SHOWTURTLE","HT","HIDETURTLE","PU","PENUP","PD","PENDOWN","REPEAT","TO","END","HOME","SETW"};

enum class Keywords {
    FD,
    BK,
    LT,
    RT,
    CS,
    ST,
    HT,
    PU,
    PD,
    REPEAT,
    TO,
    END,
    HOME,
    SETW,
    MAKE,
    PRINT,
    SETPC,
    IF,
    IFELSE,
    WHILE,
    PLACEHOLDER
};

struct Token {
    TokenType type;
    QString lexeme;
    double numberValue;
    int line;
    int column;

    Token(TokenType t = TokenType::INVALID, QString l = "",
          double n = 0.0, int ln = 0, int col = 0)
        : type(t), lexeme(l), numberValue(n), line(ln), column(col) {}
};

class Lexer {
    QString input_;
    size_t position_;
    int line_;
    int column_;

    QChar currentChar() const;
    //QChar nextChar() const;
    bool isalnum(QChar);
    void advance();
    void skipWhitespace();
    Token parseNumber();
    Token parseIdentifierOrKeyword();

public:
    Token nextToken();
    void Initialize(const QString&);
    QString tokenTypeToString(TokenType type);
    Lexer(const QString& input = "")
        : input_(input), position_(0), line_(1), column_(1) {}

};
