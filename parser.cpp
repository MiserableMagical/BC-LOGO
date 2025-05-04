/*
 * parser.cpp 解释器主体函数
 * beacon_cwk 25/02/12
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

using std::set;
using std::map;
using std::vector;

//set<QString> defaultNames = {"FD","FORWARD","BK","BACKWARD","LT","LEFT","RT","RIGHT",
//                             "CS","ST","SHOWTURTLE","HT","HIDETURTLE","PU","PENUP","PD","PENDOWN","REPEAT","TO","END","HOME","SETW"};

map<QString, Keywords> defaultNames = {
    {"FD", Keywords::FD},
    {"FORWARD",Keywords::FD},
    {"BK", Keywords::BK},
    {"BACK",Keywords::BK},
    {"LT", Keywords::LT},
    {"LEFT",Keywords::LT},
    {"RT", Keywords::RT},
    {"RIGHT",Keywords::RT},
    {"CS",Keywords::CS},
    {"ST", Keywords::ST},
    {"SHOWTURTLE",Keywords::ST},
    {"HT", Keywords::HT},
    {"HIDETURTLE",Keywords::HT},
    {"PU",Keywords::PU},
    {"PENUP",Keywords::PU},
    {"PD",Keywords::PD},
    {"PENDOWN",Keywords::PD},
    {"REPEAT",Keywords::REPEAT},
    {"TO",Keywords::TO},
    {"END",Keywords::END},
    {"HOME",Keywords::HOME},
    {"SETW",Keywords::SETW},
    {"SETWIDTH",Keywords::SETW},
    {"MAKE",Keywords::MAKE},
    {"PRINT",Keywords::PRINT},
    {"SETPC",Keywords::SETPC},
    {"AND",Keywords::PLACEHOLDER},
    {"OR",Keywords::PLACEHOLDER},
    {"IF",Keywords::IF},
    {"IFELSE",Keywords::IFELSE},
    {"WHILE",Keywords::WHILE},
    {"SETPCDEC",Keywords::SETPCDEC},
    {"SETX",Keywords::SETX},
    {"SETY",Keywords::SETY},
    {"SETXY",Keywords::SETXY},
    {"SETBG",Keywords::SETBG},
    {"LOCALMAKE",Keywords::LOCALMAKE},
    {"STOP",Keywords::STOP},
    {"WAIT",Keywords::WAIT},
    {"CLEAN",Keywords::CLEAN}
};

map<QString, Keywords> mathFunctions = {
    {"RANDOM",Keywords::RANDOM},
    {"SIN",Keywords::SIN},
    {"COS",Keywords::COS}
};

Keywords keyConvert(QString name) {
    return defaultNames[name];
}

void Wait(int milis)
{
    static timespec ns_sleep;
    ns_sleep.tv_nsec = 1000000;
    QElapsedTimer t;
    t.start();
    while(t.elapsed() < milis)
    {
        QCoreApplication::processEvents();
        nanosleep(&ns_sleep, 0);
    }
}

bool MainWindow::isVariable(QString name)
{
    return varLayer.count(name);
}

qreal MainWindow::getVariable(QString name)
{
    if(!isVariable(name)) return 0;
    int lay = varLayer[name];
    return varNames[lay][name];
}

bool MainWindow::Parser(std::vector<Token> & tokens)
{

    if(tokens.size() == 0)
        return true;
    if(rec_layers > maxRecursions)
    {
        Report("Too many recursive calls");
        return false;
    }
    if(rec_layers == stop_flag)
    {
        return true;
    }

    //text = text.trimmed();
    Token word = tokens.back();
    tokens.pop_back();
    if(word.type == TokenType::INVALID) {
        Report("Syntax Error");
        return false;
    }
    if(word.type == TokenType::END_OF_INPUT)
        return true;
    //word = word.toUpper();

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::PRINT)
    {
        if(!dealPrint(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::WHILE)
    {
        if(!dealWhile(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::IF)
    {
        if(!dealIF(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::IFELSE)
    {
        if(!dealIFELSE(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::STOP)
    {
        if(!dealSTOP(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::MAKE)
    {
        if(!dealMake(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::LOCALMAKE)
    {
        if(!dealLocalMake(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::WAIT)
    {
        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        Wait(num);
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::TO)
    {
        if(!dealTO(tokens)) return false;
        return true;
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::END) {// the string has ended
        return true;
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::REPEAT)
    {
        if(!dealRepeat(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::CS) {
        PArea->clearScreen();
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::HOME) {
        PArea->Home();
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::ST)
    {
        PArea->showTurtle();
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::HT)
    {
        PArea->hideTurtle();
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::PU)
    {
        PArea->penUp();
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::PD)
    {
        PArea->penDown();
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::FD)
    {
        if(!dealFD(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::BK)
    {
        if(!dealBK(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::LT)
    {
        if(!dealLT(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::RT)
    {
        if(!dealRT(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::SETW)
    {
        if(!dealsetW(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::SETPC)
    {
        if(!dealsetPC(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::SETPCDEC)
    {
        if(!dealsetPCdec(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::SETX)
    {
        if(!dealsetX(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::SETY)
    {
        if(!dealsetY(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::SETXY)
    {
        if(!dealsetXY(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::SETBG)
    {
        if(!dealsetBG(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::CLEAN)
    {
        PArea->Clean();
        return Parser(tokens);
    }

    if(word.type == TokenType::NUMBER || (word.type == TokenType::IDENTIFIER && isVariable(word.lexeme))
        || word.type == TokenType::LPAREN || ((word.type == TokenType::IDENTIFIER && isFunction(word.lexeme))))
    {
        tokens.push_back(word);
        bool ok;
        qreal num = getNum(tokens, ok);
        if(ok == false)
        {
            return false;
        }
        Report("Result is : " + QString::number(num));
        return Parser(tokens);
    }

    if(ProcNames.count(word.lexeme))//进入一个过程
    {
        int id = ProcNames[word.lexeme];//获取过程编号
        id--;
        int ArgNum = Args[id].size();
        bool ok = true;
        vector<qreal> nums = getNums(tokens, ok, ArgNum);//获取参数
        if(ok == false)
            return false;


        vector<Token> temp = ProcTokens[id];
        map<QString, int> varLayer_old = varLayer;
        rec_layers++;

        /*加载参数的临时变量*/
        for(int i = 0;i < ArgNum;i++)
        {
            varNames[rec_layers][Args[id][i]] = nums[i];
            varLayer[Args[id][i]] = rec_layers;
        }

        if(!Parser(temp))
            return false;

        /*还原*/
        varNames[rec_layers].clear();
        rec_layers--;
        varLayer = varLayer_old;
        stop_flag = -1;//撤销stop标记
        return Parser(tokens);
    }

    Report(word.lexeme + " is not a LOGO procedure.");
    return false;
}
