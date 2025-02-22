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
            cursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);
            pMainWind -> cmd_buf = cursor.selection().toPlainText();
            //qDebug() << pMainWind -> cmd_buf;
            pMainWind -> modifyText();
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
