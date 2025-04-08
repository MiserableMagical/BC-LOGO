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

QString version = "V0.5[250408]";
QString curText = "Beacon's Logo " + version + " for Windows x64\nWenkai Cheng\n";

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


void MainWindow::modifyText()//执行缓冲区的命令
{
    QString curstr = cmd_buf.trimmed();
    apply(curstr, 0);
}

void MainWindow::Tokenize(QString &str,std::vector<Token> &tokens)
{
    Lexer * lexer = new Lexer(str);

    Token token;
    do {
        token = lexer->nextToken();
        tokens.push_back(token);
    } while (token.type != TokenType::END_OF_INPUT &&
             token.type != TokenType::INVALID);
    delete lexer;
}

QString tokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::KEYWORD: return "KEYWORD";
    case TokenType::IDENTIFIER: return "IDENTIFIER";
    case TokenType::NUMBER: return "NUMBER";
    case TokenType::OPERATOR: return "OPERATOR";
    case TokenType::LPAREN: return "LPAREN";
    case TokenType::RPAREN: return "RPAREN";
    case TokenType::LBRACKET: return "LBRACKET";
    case TokenType::RBRACKET: return "RBRACKET";
    case TokenType::COMMA: return "COMMA";
    case TokenType::SEMICOLON: return "SEMICOLON";
    case TokenType::END_OF_INPUT: return "END_OF_INPUT";
    case TokenType::INVALID: return "INVALID";
    default: return "UNKNOWN";
    }
}

void MainWindow::apply(QString &str,bool echo)//现在echo只为false
{
    if(!Defmode)
    {
        if(echo)
            setListenerText(str);
        //singleStepParser(curstr);
        //Parser(str);
        vector<Token> tokens;
        Tokenize(str, tokens);
        std::reverse(tokens.begin(),tokens.end());
        for (const auto& t : tokens) {
            qDebug() << "Type: " << tokenTypeToString(t.type)
            << "\tLexeme: " << t.lexeme << "\tLine: " << t.line
                     << "\tColumn: " << t.column;
        }
        Parser(tokens);
        if(Defmode)
            ui->DefHint->show();
    }
    else
    {
        str = str.trimmed();
        vector<Token> curstr;
        Tokenize(str, curstr);
        /*if(!str.isEmpty() && (str.section(' ',0,0)).toUpper() == "END")
        {
            Defmode = false;
            ui->DefHint->hide();
            if(!reDef) setListenerText(Def_name + " defined.");
            else setListenerText(Def_name + " redefined.");
            return;
        }
        if(echo)
            setListenerText("> " + str);*/

        Procs[Def_id].append(str);

        vector<Token> added;
        for(auto &curToken : curstr)
        {
            if(curToken.lexeme == "END")//End of definition
            {
                Defmode = false;
                ui->DefHint->hide();
                if(!reDef) setListenerText(Def_name + " defined.");
                else setListenerText(Def_name + " redefined.");
                break;
            }
            if(curToken.type == TokenType::END_OF_INPUT) continue;
            added.push_back(curToken);
        }

        reverse(added.begin(),added.end());
        for(auto &curToken : added)
            ProcTokens[Def_id].push_back(curToken);
        //ProcTokens[Def_id].push_back(curstr);
    }
}

bool MainWindow::checkType(QString & str)//检查是否为定义语句
{
    vector<Token> tokens;
    Tokenize(str, tokens);
    for(auto &token : tokens) {
        if(token.type == TokenType::KEYWORD && token.lexeme == "TO")
            return true;
    }
    return false;
}

//加载过程
//将过程压缩成一行（除含 TO 的行）
void MainWindow::loadFile()
{
    QString fname = QFileDialog::getOpenFileName(this,"选择一个文件...",QString(),"LOGO Files(*.lgo)");
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
    //开始压缩
    QString curText;
    bool lastType = false;
    for(int i = 0;i < strs.size();i++)
    {
        bool thisType = checkType(strs[i]);
        if(thisType || lastType) {
            qDebug() << curText;
            apply(curText);
            curText = QString();
        }
        curText += strs[i] + " ";
        lastType = thisType;
        //apply(strs[i], false);
    }
    if(!curText.isEmpty())
    {
        qDebug() << curText;
        apply(curText);
        curText = QString();
    }
    /*
    for(int i = 0;i < strs.size();i++)
    {
        apply(strs[i], false);
    }*/
    //text.replace("\n"," ");
    //text.replace("\r"," ");
    //apply(text, false);
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
//新建一个编辑器
void MainWindow::launchEditor(){
    editor* edit = new editor;
    editPool.push_back(edit);
    MyHighlighter* highlighter = new MyHighlighter(edit->edit.document());
    HltPool.push_back(highlighter);
}

//用编辑器打开文件
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
    Lis->resize(900,220);
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
    connect(ui->actionClear,&QAction::triggered,Lis,&myListener::Clear);
    connect(ui->actionSelectAll,&QAction::triggered,Lis,&myListener::selectAll);
    //connect(ui->lineEdit,&QLineEdit::editingFinished,this,&MainWindow::modifyText);

    //connect(ui->textEdit,&QTextEdit::);
}

MainWindow::~MainWindow()
{
    delete ui;
}
