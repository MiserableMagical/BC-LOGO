#include "editor.h"
#include "QToolBar"
#include "QFileDialog"
#include "QSaveFile"
#include "QMessageBox"
#include "QCloseEvent"

void editor::updTitle()
{
    if(!edit.document()->isModified())
        return;
    if(curPath.isEmpty())
        this->setWindowTitle("*Untitled");
    else
        this->setWindowTitle("*" + curPath);
}

editor :: editor(){

    QToolBar * toolbar = new QToolBar(this);
    QAction *file_save = new QAction("Save", this);

    toolbar->addAction(file_save);
    //QAction *file_save = new QAction("保存", this);
    toolbar->show();
    addToolBar(toolbar);

    file_save -> setShortcut(QKeySequence("Ctrl+S"));

    this->resize(800,480);
    this->show();

    edit.setParent(this);
    edit.resize(800,460);
    edit.move(0,20);
    edit.show();
    edit.setFont((QFont){"Courier",12});

    curPath = QString();
    connect(file_save,&QAction::triggered,this,&editor::onSaveFile);
    //connect(file_save2,&QAction::triggered,this,&editor::onSaveFile);
    connect(&edit,&QTextEdit::textChanged,this,&editor::updTitle);
    //connect(file_save, &QAction::triggered, this, &editor::onSaveFile);

    QAction *Undo = toolbar->addAction("撤销");
    Undo->setShortcut(tr("Ctrl+Z"));
    connect(Undo,&QAction::triggered,this,&editor::onUndo);

    QAction *Cut = toolbar->addAction("剪切");
    Cut->setShortcut(tr("Ctrl+X"));
    connect(Cut,&QAction::triggered,this,&editor::onCut);

    QAction *Copy = toolbar->addAction("复制");
    Copy->setShortcut(tr("Ctrl+C"));
    connect(Copy,&QAction::triggered,this,&editor::onCopy);

    QAction *Paste = toolbar->addAction("粘贴");
    Paste->setShortcut(tr("Ctrl+V"));
    connect(Paste,&QAction::triggered,this,&editor::onPaste);
}

void editor::openFile(QString path)
{
    QFile file(path);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "提示", "Failed to open!");
        return;
    }
    QTextStream in(&file);
    edit.setPlainText(in.readAll());
    curPath = path;
    this->setWindowTitle(curPath);
}

void editor::closeEvent(QCloseEvent *ev)
{
    if(!edit.document()->isModified())
    {
        this->close();
    }
    else
    {
        if(caution()) {
            this->close();
        }
        ev->ignore();
    }
}

bool editor::caution()//弹出提示框
{
    const QMessageBox::StandardButton btn = QMessageBox::warning(this, "提示", "是否保存文件？", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch(btn)
    {
    case QMessageBox::Save :
        return onSaveFile();
    case QMessageBox::Cancel :
        return false;
    default : break;
    }
    return true;
}

bool editor::saveFile(QString path)
{
    QSaveFile file(path);
    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&file);
        out<<edit.toPlainText();
        if(!file.commit())
        {
            QMessageBox::warning(this, "提示", "Failed to save!");
            return false;
        }
    }
    else
        return false;
    curPath = path;
    this->setWindowTitle(curPath);
    edit.document()->setModified(false);
    return true;
}

bool editor::onSaveasFile()//弹出文件选择框
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter("LOGO(*.lgo)");
    if(dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

bool editor::onSaveFile()
{
    if(curPath.isEmpty())
        return onSaveasFile();
    return saveFile(curPath);
}

void editor::onCut()
{
    edit.cut();
}

void editor::onCopy()
{
    edit.copy();
}

void editor::onUndo()
{
    edit.undo();
}

void editor::onPaste()
{
    edit.paste();
}
