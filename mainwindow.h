#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <set>
#include "lexer.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

using std::set;
using std::map;
using std::vector;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    int rec_layers = 0;
    bool reDef = false;
    QString Def_name = QString();
    int Def_id = -1;

    map<QString, int> ProcNames;
    map<QString, qreal> varNames;
    vector<QStringList> Procs;
    vector<vector<vector<Token>>> ProcTokens;
public:
    QString cmd_buf;
    bool Defmode = false;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void launchEditor();
    void readFile();
    void openFile();
    void loadFile();
    bool saveFile(QString path);
    void onSaveasFile();
    void modifyText();
    void apply(QString &str,bool echo = true);
    void setListenerText(QString str);
    void initPArea();
    void initListener();

    //parser
    void singleStepParser(QString text);
    bool Parser(std::vector<Token> &);
    double getNum(std::vector<Token>&, bool&);
    void Tokenize(QString &,std::vector<Token> &);
    void getExpBound(std::vector<Token>&,std::vector<Token>&,int&);
    qreal eval(vector<Token>&,bool &isValid);

    //separate functions
    bool dealPrint(vector<Token>&);
    bool dealsetPC(vector<Token>&);
    bool dealLT(vector<Token>&);
    bool dealRT(vector<Token>&);
    bool dealsetW(vector<Token>&);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
