#include "mainwindow.h"
#include "mylistener.h"
#include "QTextCharFormat"

using std::set;
using std::map;
extern map<QString,Keywords> defaultNames;

MyHighlighter::MyHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    // 定义关键字的格式
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    // 定义注释的格式
    commentFormat.setForeground(Qt::darkGreen);
    commentFormat.setFontItalic(true);

    numFormat.setForeground((QColor){110,60,0});//Brown
    brkFormat.setForeground(Qt::red);

    // 定义字符串的格式
    quotationFormat.setForeground(Qt::darkRed);

    // 添加关键字高亮规则
    QStringList keywordPatterns;
    //keywordPatterns << "\\bif\\b" << "\\belse\\b" << "\\bwhile\\b"
    //                << "\\breturn\\b" << "\\bclass\\b" << "\\bvoid\\b";
    for(auto &str : defaultNames)
    {
        keywordPatterns << "\\b" + str.first + "\\b";
    }
    //keywordPatterns << "\\bREPEAT\\b" << "\\bTO\\b" << "";

    foreach (const QString &pattern, keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    //Highlight for numbers
    HighlightingRule numRule;
    numRule.pattern = QRegularExpression("[0-9]");
    numRule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    numRule.format = numFormat;
    highlightingRules.append(numRule);

    //Highlight for brackets
    HighlightingRule brkRule;
    brkRule.pattern = QRegularExpression("[\\[\\]]");
    brkRule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    brkRule.format = brkFormat;
    highlightingRules.append(brkRule);

    // 添加注释高亮规则
    /*HighlightingRule commentRule;
    commentRule.pattern = QRegularExpression("//[^\n]*");
    commentRule.format = commentFormat;
    highlightingRules.append(commentRule);

    // 添加字符串高亮规则
    HighlightingRule quotationRule;
    quotationRule.pattern = QRegularExpression("\".*\"");
    quotationRule.format = quotationFormat;
    highlightingRules.append(quotationRule);*/
}

void MyHighlighter::highlightBlock(const QString &text) {
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
