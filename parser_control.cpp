/*
 * parser_control.cpp 处理控制语句
 * beacon_cwk 25/04/23
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

bool Truth(qreal val);

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
    for(int i = 1;i <= std::min(num, (double)repLim);i++) {
        if(!Parser(repPat)) return false;
        repPat = repPat_2;
    }
    return true;
}

bool MainWindow::dealSTOP(std::vector<Token> & tokens)
{
    if(rec_layers != 0)
        stop_flag = rec_layers;
    return true;
}
