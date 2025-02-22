#include "mainwindow.h"
#include "QFileDialog"
#include "ui_mainwindow.h"
#include "QFile"
#include "QLineEdit"
#include "paint.h"
#include "editor.h"
#include "QSaveFile"
#include "QMessageBox"
#include "mylistener.h"

QString curText = "Beacon's Logo V0.1.0 for Windows x64\nWenkai Cheng\n";

myListener *Lis;

void MainWindow::setListenerText(QString str)
{
    //ui->Listener->setText(str);
    //ui->Listener->append(str);
    //ui->Listener->moveCursor(QTextCursor::End);
    //ui->Listener->ensureCursorVisible();
    //Lis->append(str);
    QTextCursor cursor = Lis->textCursor();
    cursor.insertText(str);
    Lis->append("");
    Lis->moveCursor(QTextCursor::End);
    Lis->ensureCursorVisible();
}

/*void MainWindow::modifyText()
{
    QString curstr = ui->lineEdit->text();
    curText += curstr;
    curText += "\n";
    ui->lineEdit->setText("");
    apply(curstr);
}*/


void MainWindow::modifyText()//执行命令，作用同apply
{
    QString curstr = cmd_buf.trimmed();
    apply(curstr, 0);
}


void MainWindow::apply(QString &str,bool echo)//现在echo只为false
{
    if(!Defmode)
    {
        if(echo)
            setListenerText(str);
        //singleStepParser(curstr);
        Parser(str);
        if(Defmode)
            ui->DefHint->show();
    }
    else
    {
        str = str.trimmed();
        if(!str.isEmpty() && (str.section(' ',0,0)).toUpper() == "END")
        {
            Defmode = false;
            ui->DefHint->hide();
            if(!reDef) setListenerText(Def_name + " defined.");
            else setListenerText(Def_name + " redefined.");
            return;
        }
        if(echo)
            setListenerText("> " + str);
        Procs[Def_id].append(str);
    }
}

void MainWindow::loadFile()
{
    QString fname = QFileDialog::getOpenFileName(this,"选择一个文件...",QString(),"LOGO Files(*.lgo)");
    //ui->Listener->setText("777");
    QFile file(fname);
    if(!file.exists())
    {
        setListenerText("File open failed");
        return;
    }
    setListenerText("Loading from " + fname);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray allLines = file.readAll();
    QString text(allLines);
    QStringList strs = text.split('\n');
    for(int i = 0;i < strs.size();i++)
    {
        apply(strs[i], false);
    }
}

//save all procedures
bool MainWindow::saveFile(QString path)
{
    QSaveFile file(path);
    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&file);
        //out<<edit.toPlainText();
        for(auto &curProc : ProcNames)
        {
            out<<"TO "<<curProc.first<<'\n';
            int idx = curProc.second - 1;
            for(int i = 0;i < Procs[idx].size();i++)
            {
                out<<Procs[idx][i]<<'\n';
            }
            out<<"END\n";
        }
        if(!file.commit())
        {
            QMessageBox::warning(this, "提示", "Failed to save!");
            return false;
        }
    }
    else
        return false;
    return true;
}

void MainWindow::onSaveasFile()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter("LOGO(*.lgo)");
    if(dialog.exec() != QDialog::Accepted)
        return;
    QString path = dialog.selectedFiles().first();
    if(saveFile(path))
        setListenerText("Saving workspace into file " + path);
}

std::vector<editor*> editPool;
std::vector<MyHighlighter*> HltPool;
void MainWindow::launchEditor(){
    editor* edit = new editor;
    editPool.push_back(edit);
    MyHighlighter* highlighter = new MyHighlighter(edit->edit.document());
    HltPool.push_back(highlighter);
}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this,"选择一个文件...",QString(),"LOGO Files(*.lgo)");
    if(!path.isEmpty())
    {
        editor* edit = new editor;
        edit->openFile(path);
        editPool.push_back(edit);
        MyHighlighter* highlighter = new MyHighlighter(edit->edit.document());
        HltPool.push_back(highlighter);
    }
}

PaintArea *PArea;
void MainWindow::initPArea()
{
    PArea = new PaintArea;
    PArea->init(this);
}

MyHighlighter* highlighter;
void MainWindow::initListener()
{
    Lis = new myListener;
    Lis->setParent(this);
    Lis->resize(920,220);
    Lis->move(40,498);
    Lis->register_key_enter_handler(this);
    Lis->show();
    highlighter = new MyHighlighter(Lis->document());
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->Listener->setFont((QFont){"Courier",12});
    ui->lineEdit->setFont((QFont){"Courier",12});
    ui->lineEdit->setFixedHeight(18);
    ui->Listener->hide();
    ui->lineEdit->hide();
    ui->DefHint->hide();

    setWindowTitle("Beacon's LOGO(BC LOGO)");
    initPArea();
    initListener();
    setListenerText(curText);

    connect(ui->actionOpen,&QAction::triggered,this,&MainWindow::openFile);
    connect(ui->actionNew,&QAction::triggered,this,&MainWindow::launchEditor);
    connect(ui->actionSave,&QAction::triggered,this,&MainWindow::onSaveasFile);
    connect(ui->actionLoad,&QAction::triggered,this,&MainWindow::loadFile);

    //connect(ui->lineEdit,&QLineEdit::editingFinished,this,&MainWindow::modifyText);

    //connect(ui->textEdit,&QTextEdit::);
}

MainWindow::~MainWindow()
{
    delete ui;
}
