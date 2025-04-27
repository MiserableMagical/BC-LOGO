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

const int maxRecursions = 300;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    int rec_layers = 0;
    int stop_flag = -1;
    bool reDef = false;
    QString Def_name = QString();
    int Def_id = -1;

    map<QString, int> ProcNames;
    vector<vector<QString>> Args;
    map<QString, qreal> varNames[maxRecursions + 5];
    map<QString, int> varLayer;
    vector<QStringList> Procs;
    vector<vector<Token>> ProcTokens;
public:
    QString cmd_buf;
    bool Defmode = false;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void launchEditor();
    void About();
    void showHelp();
    void savePic();
    void readFile();
    void openFile();
    void loadFile();
    bool saveFile(QString path);
    void onSaveasFile();
    void bufferExec();
    void apply(QString &str,bool echo = false);
    void setListenerText(QString str);
    void initPArea();
    void initListener();


private:
    //parser
    void singleStepParser(QString text);
    bool checkType(QString &);
    bool Parser(std::vector<Token> &);
    double getNum(std::vector<Token>&, bool&);
    vector<qreal> getNums(std::vector<Token>&, bool&, int);
    void Tokenize(QString &,std::vector<Token> &);
    void getExpBound(std::vector<Token>&,std::vector<Token>&,int&);
    vector<qreal> eval(vector<Token>&,bool &isValid);
    bool isVariable(QString);
    bool isFunction(QString);
    qreal getVariable(QString);
    //separate functions
    bool dealPrint(vector<Token>&);
    bool dealsetPC(vector<Token>&);
    bool dealLT(vector<Token>&);
    bool dealRT(vector<Token>&);
    bool dealsetW(vector<Token>&);
    bool dealsetX(vector<Token>&);
    bool dealsetY(vector<Token>&);
    bool dealsetXY(vector<Token>&);
    bool dealRepeat(vector<Token>&);
    bool dealFD(vector<Token>&);
    bool dealBK(vector<Token>&);
    bool dealIF(vector<Token>&);
    bool dealTO(vector<Token>&);
    bool dealIFELSE(vector<Token>&);
    bool dealMake(vector<Token>&);
    bool dealLocalMake(vector<Token>&);
    bool dealWhile(vector<Token>&);
    bool dealSTOP(vector<Token>&);
    bool dealsetPCdec(vector<Token>&);
    bool dealsetBG(vector<Token>&);
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
