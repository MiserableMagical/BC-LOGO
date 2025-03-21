#include "mylistener.h"
#include <QDebug>
#include <QKeyEvent>
#include <QEvent>
#include <QTextDocumentFragment>

myListener::myListener() {
    this->setFont((QFont){"Courier",12});
    //connect(this, &myListener::cursorPositionChanged, this, &myListener::cursorLock);
}

void myListener::register_key_enter_handler(MainWindow* pWindow)
{
    this->pWindow = pWindow;
}

void myListener::keyReleaseEvent(QKeyEvent* e)//回车
{
    MainWindow* pMainWind = NULL;
    if (e->key() == Qt::Key_Return)
    {
        if (this->pWindow)
        {
            //qDebug() <<"Event triggered";
            pMainWind = (MainWindow*)pWindow;
            totLines++;

            QTextCursor cursor = this->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            //试图将光标后的内容移回原位
            auto tmpText = cursor.selection().toPlainText();
            if(!tmpText.isEmpty()) cursor.removeSelectedText();
            cursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);

            pMainWind -> cmd_buf = cursor.selection().toPlainText() + tmpText;

            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            if(!tmpText.isEmpty()) cursor.insertText(tmpText + '\n');

            pMainWind -> modifyText();
            //qDebug() << pMainWind -> cmd_buf;
        }
    }
}

//Lock the cursor in the last row
void myListener::cursorLock()
{
    QTextDocument *doc = this->document();
    QTextCursor cursor = this->textCursor();
    //qDebug()<<cursor.blockNumber();
    if(cursor.blockNumber() + 1 < doc->blockCount())
    {
        cursor.movePosition(QTextCursor::End);
        this->setTextCursor(cursor);
    }
}
