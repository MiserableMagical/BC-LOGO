#ifndef MYLISTENER_H
#define MYLISTENER_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QMainWindow>
#include "mainwindow.h"
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QRegularExpression>

class MyHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    MyHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat numFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat brkFormat;
};

class myListener : public QTextEdit
{
    Q_OBJECT
public:
    myListener();
    void register_key_enter_handler(MainWindow* pWindow);
protected:
    void keyReleaseEvent(QKeyEvent *e) override;
    void cursorLock();
    void Highlight();
    MainWindow* pWindow;
    int totLines = 1;
};

#endif // MYLISTENER_H
