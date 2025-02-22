#ifndef EDITOR_H
#define EDITOR_H

#include "QTextEdit"
#include "QMainWindow"

class editor : public QMainWindow
{
    Q_OBJECT

public:
    QTextEdit edit;
    QString curPath;
    editor();
    void openFile(QString path);
    bool saveFile(QString path);
    bool onSaveFile();
    bool onSaveasFile();
    void updTitle();
    void onClose();
    bool caution();
    void onUndo();
    void onCopy();
    void onPaste();
    void onCut();
    void closeEvent(QCloseEvent *ev);
};

#endif // EDITOR_H
