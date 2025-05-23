/*
 * highlight.cpp 语法高亮
 * beacon_cwk 25/02/14
*/

#include "mainwindow.h"
#include "mylistener.h"
#include "QTextCharFormat"

using std::set;
using std::map;
extern map<QString,Keywords> defaultNames;
extern map<QString,Keywords> mathFunctions;

MyHighlighter::MyHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    // 定义关键字的格式
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    //数字
    numFormat.setForeground((QColor){110,60,0});//Brown
    //括号
    brkFormat.setForeground(Qt::red);
    //符号
    opFormat.setForeground(Qt::darkBlue);

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
    for(auto &str : mathFunctions)
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
    brkRule.pattern = QRegularExpression("[\\[\\]()]");
    brkRule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    brkRule.format = brkFormat;
    highlightingRules.append(brkRule);

    //Highlight for operators
    HighlightingRule opRule;
    opRule.pattern = QRegularExpression("[\\+\\-\\*\\/\\<\\>\\=]");
    opRule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    opRule.format = opFormat;
    highlightingRules.append(opRule);

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
