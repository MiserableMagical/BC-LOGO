/*
 * mainwindow.cpp
 * beacon_cwk 24/12/09
*/

#include "mainwindow.h"
#include "QFileDialog"
#include "ui_mainwindow.h"
#include "QFile"
#include "paint.h"
#include "editor.h"
#include "QSaveFile"
#include "QMessageBox"
#include "mylistener.h"
#include "QDesktopServices"
#include "QCryptographicHash"
#include "QImage"

#include "core/engine.h"
#include "core/source.h"
#include "qtconsole.h"
#include "qtplotter.h"

QString version = "V2.0-RC";
QString curText = "Beacon's Logo " + version + " for Windows x64\nWenkai Cheng\n";

myListener *Lis;
PaintArea *PArea = nullptr;

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

QString MainWindow::transcript() const
{
    return Lis ? Lis->toPlainText() : QString();
}

void MainWindow::clearTranscript()
{
    if (Lis) Lis->Clear();
}

QString MainWindow::stateSnapshot() const
{
    const auto num = [](double v) {
        if (qAbs(v) < 5e-7) v = 0.0;   // 抹掉 cos(-pi/2) 之类的浮点噪声
        return QString::number(v, 'f', 6);
    };

    const QImage image = PArea->getPixmap()->toImage().convertToFormat(QImage::Format_ARGB32);
    const QByteArray bytes(reinterpret_cast<const char *>(image.constBits()),
                           image.sizeInBytes());
    const QByteArray digest = QCryptographicHash::hash(bytes, QCryptographicHash::Sha1).toHex();

    QString s;
    s += "x=" + num(PArea->turtleX()) + "\n";
    s += "y=" + num(PArea->turtleY()) + "\n";
    s += "angle=" + num(PArea->turtleAngleDeg()) + "\n";
    s += QString("pen=%1\n").arg(PArea->penIsDown() ? 1 : 0);
    s += QString("width=%1\n").arg(PArea->penWidth());
    s += "color=" + PArea->penColor().name(QColor::HexArgb) + "\n";
    s += "bg=" + PArea->backgroundColor().name(QColor::HexArgb) + "\n";
    s += "image=sha1:" + QString::fromLatin1(digest) + "\n";
    return s;
}

void MainWindow::bufferExec()//执行缓冲区的命令
{
    QString curstr = cmd_buf.trimmed();
    apply(curstr, 0);
}

void MainWindow::apply(QString &str, bool echo)
{
    if (echo)
        setListenerText(str);

    if (!engine_)
        return;

    engine_->runLine(str);
    ui->DefHint->setVisible(engine_->isDefining());
}

void MainWindow::runScript(const QString &name, const QString &text)
{
    if (!engine_)
        return;

    const logo::Source source(name, text);
    engine_->runSource(source);
    ui->DefHint->setVisible(engine_->isDefining());
}

void MainWindow::loadFile()
{
    const QString fname = QFileDialog::getOpenFileName(
        this, "选择一个文件...", QString(), "LOGO Files(*.lgo)");
    if (fname.isEmpty())
        return;

    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setListenerText("File open failed");
        return;
    }

    setListenerText("Loading from " + fname);
    const QString text = QString::fromUtf8(file.readAll());
    runScript(fname, text);
}

//save all procedures
bool MainWindow::saveFile(QString path)
{
    if (!engine_)
        return false;

    QSaveFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return false;

    QTextStream out(&file);
    const auto &definitions = engine_->definitions();
    for (auto it = definitions.cbegin(); it != definitions.cend(); ++it) {
        out << it.value();
        if (!it.value().endsWith(QLatin1Char('\n')))
            out << '\n';
        out << '\n';
    }

    if (!file.commit()) {
        QMessageBox::warning(this, "提示", "Failed to save!");
        return false;
    }
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

void MainWindow::About()
{
    QWidget * aboutWindow = new QWidget;
    aboutWindow->resize(500,250);
    aboutWindow->show();
    QLabel * text = new QLabel(aboutWindow), * image1 = new QLabel(aboutWindow), *image2 = new QLabel(aboutWindow);

    image1->move(135,20);
    image1->resize(75,75);
    image1->show();
    image1->setPixmap(QPixmap(":/new/cursor/ICON2.jpg"));
    image1->setScaledContents(true);

    image2->move(290,20);
    image2->resize(75,75);
    image2->show();
    image2->setPixmap(QPixmap(":/new/cursor/beacon.jpg"));
    image2->setScaledContents(true);

    text->move(100,100);
    static QString aboutText = "Beacon's LOGO(BC LOGO) " + version + '\n'
        + "Author: beacon_cwk\n"
        + "QQ/Email: 2733630473@qq.com\n"
        + "For study methods\n"
        + "Thank you for trying this software!";
    text->setText(aboutText);
    text->setAlignment(Qt::AlignCenter);
    text->setFont((QFont){"Consolas",12});
    text->show();
}

void MainWindow::showHelp()
{
    QString pdfPath = qApp->applicationDirPath() + "/docs/logo-reference.pdf";
    QDesktopServices::openUrl(QUrl::fromLocalFile(pdfPath));
}

void MainWindow::savePic()
{
    QPixmap *pixmap = PArea->getPixmap();

    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter("JPEG(*.jpg)");
    if(dialog.exec() != QDialog::Accepted)
        return;

    QString filePath = dialog.selectedFiles().first();
    if (!pixmap->save(filePath, "JPEG")) {
        setListenerText("Failed to save");
    } else {
        setListenerText("Successfully saved as " + filePath);
    }
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

    plotter_ = std::make_unique<QtPlotter>(PArea);
    console_ = std::make_unique<QtConsole>(this);
    engine_ = std::make_unique<logo::Engine>(*console_, *plotter_);

    setListenerText(curText);

    connect(ui->actionOpen,&QAction::triggered,this,&MainWindow::openFile);
    connect(ui->actionNew,&QAction::triggered,this,&MainWindow::launchEditor);
    connect(ui->actionSave,&QAction::triggered,this,&MainWindow::onSaveasFile);
    connect(ui->actionLoad,&QAction::triggered,this,&MainWindow::loadFile);
    connect(ui->actionSavePic,&QAction::triggered,this,&MainWindow::savePic);

    connect(ui->actionClear,&QAction::triggered,Lis,&myListener::Clear);
    connect(ui->actionSelectAll,&QAction::triggered,Lis,&myListener::selectAll);

    connect(ui->actionAbout,&QAction::triggered,this,&MainWindow::About);
    connect(ui->actionShowHelp,&QAction::triggered,this,&MainWindow::showHelp);
    //connect(ui->actionShowHelp,&QAction::triggered,this,&MainWindow::darkMode);
    //connect(ui->actionShowHelp,&QAction::triggered,this,&MainWindow::lightMode);
    //connect(ui->lineEdit,&QLineEdit::editingFinished,this,&MainWindow::modifyText);
}

MainWindow::~MainWindow()
{
    delete ui;
}
