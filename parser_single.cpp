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

//找到该表达式的范围
void MainWindow::getExpBound(std::vector<Token> & tokens,std::vector<Token> & exp,int &result)
{

    bool nonEmpty = false;
    while(!tokens.empty() && ((tokens.back().type == TokenType::NUMBER
                                || tokens.back().type == TokenType::LPAREN
                                || tokens.back().type == TokenType::RPAREN
                                || tokens.back().type == TokenType::OPERATOR
                                || tokens.back().type == TokenType::IDENTIFIER)))
    {
        nonEmpty = true;
        if(tokens.back().type == TokenType::IDENTIFIER) {// check if it's a variable
            if(!varNames.count(tokens.back().lexeme)) break;
        }
        exp.push_back(tokens.back());
        tokens.pop_back();
    }
    if(nonEmpty == false) {
        Report("This procedure needs more input(s)");
        result = -1;
        return;
    }
}

//计算下一个参数的数值
double MainWindow::getNum(std::vector<Token> & tokens, bool &ok)
{
    ok = true;
    std::vector<Token> exp;
    int state = 0;
    getExpBound(tokens, exp, state);
    if(state == -1) {
        ok = false;
        return 0;
    }
    vector<qreal> res = eval(exp, ok);
    if(ok) return res.front();
    return 0;
}

//一次性获取多个参数的值
vector<qreal> MainWindow::getNums(std::vector<Token>& tokens, bool& ok, int cap)
{
    ok = true;
    std::vector<Token> exp;
    int state = 0;
    getExpBound(tokens, exp, state);
    if(state == -1) {
        ok = false;
        return {};
    }
    vector<qreal> res = eval(exp, ok);
    if(ok) {
        if(res.size() < cap)
        {
            Report("This procedure needs more input(s)");
            ok = false;
            return {};
        }
        return res;
    }
    return {};
}


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

bool MainWindow::dealRepeat(std::vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    num = floor(num + 1e-12);
    if(ok == false)
        return false;

    if(tokens.empty() || tokens.back().type != TokenType::LBRACKET) {
        Report("Left bracket not found");
        return false;
    }
    std::vector<Token> repPat;
    tokens.pop_back();
    int layer = 1;
    while(!tokens.empty())
    {
        auto curToken = tokens.back();
        tokens.pop_back();
        switch(curToken.type)
        {
        case TokenType::LBRACKET: layer++;repPat.push_back(curToken);break;
        case TokenType::RBRACKET: layer--;repPat.push_back(curToken);break;
        default:
            repPat.push_back(curToken);
            break;
        }
        if(layer == 0)
            break;
    }
    repPat.pop_back();
    reverse(repPat.begin(), repPat.end());

    vector<Token> repPat_2 = repPat;
    for(int i = 1;i <= num;i++) {
        if(!Parser(repPat)) return false;
        repPat = repPat_2;
    }
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

bool MainWindow::dealIF(vector<Token> & tokens)
{
    bool ok;
    double truthValue = getNum(tokens, ok);
    if(ok == false)
        return false;
    if(tokens.empty() || tokens.back().type != TokenType::LBRACKET) {
        Report("Left bracket not found");
        return false;
    }
    std::vector<Token> body;
    tokens.pop_back();
    int layer = 1;
    while(!tokens.empty())
    {
        auto curToken = tokens.back();
        tokens.pop_back();
        switch(curToken.type)
        {
        case TokenType::LBRACKET: layer++;body.push_back(curToken);break;
        case TokenType::RBRACKET: layer--;body.push_back(curToken);break;
        default:
            body.push_back(curToken);
            break;
        }
        if(layer == 0)
            break;
    }
    body.pop_back();
    reverse(body.begin(), body.end());
    if(Truth(truthValue))
    {
        if(!Parser(body)) return false;
    }
    return true;
}

bool MainWindow::dealIFELSE(vector<Token> & tokens)
{
    bool ok;
    double truthValue = getNum(tokens, ok);
    if(ok == false)
        return false;
    if(tokens.empty() || tokens.back().type != TokenType::LBRACKET) {
        Report("Left bracket not found");
        return false;
    }
    std::vector<Token> body1,body2;
    tokens.pop_back();
    //第一段
    int layer = 1;
    while(!tokens.empty())
    {
        auto curToken = tokens.back();
        tokens.pop_back();
        switch(curToken.type)
        {
        case TokenType::LBRACKET: layer++;body1.push_back(curToken);break;
        case TokenType::RBRACKET: layer--;body1.push_back(curToken);break;
        default:
            body1.push_back(curToken);
            break;
        }
        if(layer == 0)
            break;
    }

    //第二段
    if(tokens.empty() || tokens.back().type != TokenType::LBRACKET) {
        Report("Left bracket not found");
        return false;
    }
    tokens.pop_back();

    layer = 1;
    while(!tokens.empty())
    {
        auto curToken = tokens.back();
        tokens.pop_back();
        switch(curToken.type)
        {
        case TokenType::LBRACKET: layer++;body2.push_back(curToken);break;
        case TokenType::RBRACKET: layer--;body2.push_back(curToken);break;
        default:
            body2.push_back(curToken);
            break;
        }
        if(layer == 0)
            break;
    }

    if(Truth(truthValue))
    {
        body1.pop_back();
        reverse(body1.begin(), body1.end());
        if(!Parser(body1)) return false;
    }
    else
    {
        body2.pop_back();
        reverse(body2.begin(), body2.end());
        if(!Parser(body2)) return false;
    }

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
    if(defaultNames.count(name) || varNames.count(name))
    {
        Report(name + " is already in use.Try a different name.");
        return false;
    }
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
        //ProcTokens.push_back(vector<vector<Token>>());
    }
    else
    {
        Procs[reg_id - 1] = QStringList();
        ProcTokens[reg_id - 1] = vector<Token>();
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

    varNames[name] = num;
    return true;
}

const int repLim = 1000000;

bool MainWindow::dealWhile(std::vector<Token> & tokens)
{
    bool ok;
    //double num = getNum(tokens, ok);

    std::vector<Token> exp;
    int state = 0;
    getExpBound(tokens, exp, state);
    if(state == -1)
        return false;

    if(tokens.empty() || tokens.back().type != TokenType::LBRACKET) {
        Report("Left bracket not found");
        return false;
    }
    std::vector<Token> repPat;
    tokens.pop_back();
    int layer = 1;
    while(!tokens.empty())
    {
        auto curToken = tokens.back();
        tokens.pop_back();
        switch(curToken.type)
        {
        case TokenType::LBRACKET: layer++;repPat.push_back(curToken);break;
        case TokenType::RBRACKET: layer--;repPat.push_back(curToken);break;
        default:
            repPat.push_back(curToken);
            break;
        }
        if(layer == 0)
            break;
    }
    repPat.pop_back();
    reverse(repPat.begin(), repPat.end());

    vector<Token> repPat_2 = repPat;
    int repNum = 0;
    while(true) {
        ok = true;
        vector<qreal> res = eval(exp, ok);
        if(ok == false || res.empty()) return false;
        if(!Truth(res[0])) break;
        if(!Parser(repPat)) return false;
        repPat = repPat_2;
        repNum++;
        if(repNum >= repLim) break;
    }
    return true;
}

/*以上为关键字读入处理部分*/
