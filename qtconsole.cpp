#include "qtconsole.h"

#include "mainwindow.h"

QtConsole::QtConsole(MainWindow *window)
    : window_(window)
{
}

void QtConsole::print(const QString &text)
{
    if (window_)
        window_->setListenerText(text);
}
