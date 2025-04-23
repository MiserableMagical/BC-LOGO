/*
 * parser_single.cpp 处理控制以外的语句
 * beacon_cwk 25/04/02
*/

#include "QFileDialog"
#include "QFile"
#include "QLineEdit"
#include "paint.h"
#include "mainwindow.h"
#include "QRegularExpression"
#include "QTimer"
#include<set>

#define Report setListenerText
#define RegExp QRegularExpression

extern PaintArea *PArea;
extern map<QString, Keywords> defaultNames;

bool checkBrackets(QString &text)
{
    int cur = 0;
    for(int i = 0;i < text.size();i++)
    {
        if(text[i] == '[') cur++;
        if(text[i] == ']') cur--;
        if(cur < 0) return false;
    }
    return cur == 0;
}

const qreal Epsilon = 1e-11;
bool Truth(qreal val);

/*double MainWindow::getNum(std::vector<Token> & tokens, bool &ok)
{
    ok = true;
    if(tokens.size() == 0 || tokens.back().type != TokenType::NUMBER) {
        Report("This procedure needs more input(s)");
        ok = false;
        return 0;
    }
    double res = tokens.back().numberValue;
    tokens.pop_back();
    return res;
}*/

/*以下为关键字读入处理部分（模块化）*/

bool MainWindow::dealPrint(std::vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    Report(QString::number(num));
    return true;
}

bool MainWindow::dealsetPC(std::vector<Token> & tokens)
{
    if(tokens.empty() || tokens.back().type != TokenType::IDENTIFIER)
    {
        Report("This procedure(SETPC) needs more input(s)");
        return false;
    }
    Token Word = tokens.back();
    tokens.pop_back();
    QString word = Word.lexeme;

    QColor color(word);
    PArea->setPC(color);
    return true;
}

bool MainWindow::dealsetPCdec(std::vector<Token> & tokens)
{
    bool ok;
    vector<qreal> res = getNums(tokens, ok, 3);
    if(ok == false)
        return false;

    QColor color((int)res[0], (int)res[1], (int)res[2]);
    PArea->setPC(color);
    return true;
}

bool MainWindow::dealsetW(std::vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    PArea->setW(num);
    return true;
}

bool MainWindow::dealsetX(std::vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    PArea->setX(num);
    return true;
}

bool MainWindow::dealsetY(std::vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    PArea->setY(num);
    return true;
}

bool MainWindow::dealsetXY(std::vector<Token> & tokens)
{
    bool ok;
    vector<qreal> res = getNums(tokens, ok, 2);
    if(ok == false)
        return false;
    PArea->setXY(res[0], res[1]);
    return true;
}

bool MainWindow::dealLT(std::vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    PArea->turnLeft(num);
    return true;
}

bool MainWindow::dealRT(std::vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    PArea->turnRight(num);
    return true;
}

bool MainWindow::dealFD(vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    PArea->Forward(num);
    return true;
}

bool MainWindow::dealBK(vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    PArea->Backward(num);
    return true;
}

bool MainWindow::dealTO(vector<Token> & tokens)
{
    if(tokens.empty())
    {
        Report("This procedure(TO) needs more input(s)");
        return false;
    }
    Token nextToken = tokens.back();
    tokens.pop_back();
    if(!(nextToken.type == TokenType::KEYWORD || nextToken.type == TokenType::IDENTIFIER))
    {
        Report("This procedure(TO) needs more input(s)");
        return false;
    }
    QString name = nextToken.lexeme;
    name = name.toUpper();
    if(defaultNames.count(name) || isVariable(name))
    {
        Report(name + " is already in use.Try a different name.");
        return false;
    }
    //寻找参数
    vector<QString> Argnames;
    while(!tokens.empty() && tokens.back().type == TokenType::IDENTIFIER)
    {
        Argnames.push_back(tokens.back().lexeme);
        tokens.pop_back();
    }

    qDebug() << "owo" << Argnames.size();

    int reg_id = ProcNames.size() + 1;
    reDef = false;
    if(ProcNames.count(name))
    {
        reg_id = ProcNames[name];
        reDef = true;
    }
    ProcNames[name] = reg_id;
    if(!reDef)
    {
        Procs.push_back(QStringList());
        ProcTokens.push_back(vector<Token>());
        Args.push_back(Argnames);
        //ProcTokens.push_back(vector<vector<Token>>());
    }
    else
    {
        Procs[reg_id - 1] = QStringList();
        ProcTokens[reg_id - 1] = vector<Token>();
        Args[reg_id - 1] = Argnames;
        //ProcTokens[reg_id - 1] = vector<vector<Token>>();
    }
    Defmode = true;
    Def_name = name;
    Def_id = reg_id - 1;
    return true;
}

bool MainWindow::dealMake(vector<Token> & tokens)
{
    if(tokens.empty())
    {
        Report("This procedure(MAKE) needs more input(s)");
        return false;
    }
    Token nextToken = tokens.back();
    tokens.pop_back();
    if(!(nextToken.type == TokenType::KEYWORD || nextToken.type == TokenType::IDENTIFIER))
    {
        Report("This procedure(MAKE) needs more input(s)");
        return false;
    }
    QString name = nextToken.lexeme;
    name = name.toUpper();
    if(defaultNames.count(name) || ProcNames.count(name))
    {
        Report(name + " is already in use.Try a different name.");
        return false;
    }

    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;

    varNames[0][name] = num;
    if(!varLayer.count(name)) varLayer[name] = 0;
    return true;
}

bool MainWindow::dealLocalMake(vector<Token> & tokens)
{
    qDebug() << "???";
    if(tokens.empty())
    {
        Report("This procedure(LOCALMAKE) needs more input(s)");
        return false;
    }
    Token nextToken = tokens.back();
    tokens.pop_back();
    if(!(nextToken.type == TokenType::KEYWORD || nextToken.type == TokenType::IDENTIFIER))
    {
        Report("This procedure(LOCALMAKE) needs more input(s)");
        return false;
    }

    QString name = nextToken.lexeme;
    name = name.toUpper();
    if(defaultNames.count(name) || ProcNames.count(name))
    {
        Report(name + " is already in use.Try a different name.");
        return false;
    }

    bool ok;
    double num = getNum(tokens, ok);
    qDebug() << name << ' ' << num;
    if(ok == false)
        return false;

    varNames[rec_layers][name] = num;
    varLayer[name] = rec_layers;
    return true;
}
