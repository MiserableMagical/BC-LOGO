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
    {"BACKWARD",Keywords::BK},
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
    {"SETXY",Keywords::SETXY}
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

bool MainWindow::Parser(std::vector<Token> & tokens)
{

    if(tokens.size() == 0)
        return true;
    if(rec_layers > 100)
    {
        Report("Too many recursive calls");
        return false;
    }

    //text = text.trimmed();
    Token word = tokens.back();
    qDebug() << word.lexeme << (int)word.type;
    tokens.pop_back();
    if(word.type == TokenType::INVALID) {
        Report("Syntax Error");
        return false;
    }
    if(word.type == TokenType::END_OF_INPUT)
        return true;
    //word = word.toUpper();

    if(word.type == TokenType::KEYWORD && word.lexeme == "PRINT")
    {
        if(!dealPrint(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "WHILE")
    {
        if(!dealWhile(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "IF")
    {
        if(!dealIF(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "IFELSE")
    {
        if(!dealIFELSE(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "MAKE")
    {
        if(!dealMake(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "WAIT")
    {
        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        Wait(num);
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "TO")
    {
        if(!dealTO(tokens)) return false;
        return true;
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::END) {// the string has ended
        return true;
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "REPEAT")
    {
        if(!dealRepeat(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "CS") {
        PArea->clearScreen();
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "HOME") {
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

    if(word.type == TokenType::NUMBER || (word.type == TokenType::IDENTIFIER && varNames.count(word.lexeme)) || word.type == TokenType::LPAREN)
    {
        tokens.push_back(word);
        bool ok;
        qreal num = getNum(tokens, ok);
        if(ok == false)
        {
            Report("Invalid Expression");
            return false;
        }
        Report("Result is : " + QString::number(num));
        return Parser(tokens);
    }

    if(ProcNames.count(word.lexeme))//a process(without parameters)
    {
        rec_layers++;
        int id = ProcNames[word.lexeme];
        id--;
        qDebug() << "?";
        qDebug() << ProcTokens[id].size();
        for(auto &x : ProcTokens[id]) qDebug() << x.lexeme << ' '<<(int)x.type;

        vector<Token> temp = ProcTokens[id];
        if(!Parser(temp))
            return false;
        rec_layers--;
        return Parser(tokens);
    }

    Report(word.lexeme + " is not a LOGO procedure.");
    return false;
}
