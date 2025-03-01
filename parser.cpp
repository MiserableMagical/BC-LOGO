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
    {"SETW",Keywords::SETW}
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

double MainWindow::getNum(std::vector<Token> & tokens, bool &ok)
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

    if(word.type == TokenType::KEYWORD && word.lexeme == "WAIT")
    {
        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        Wait(num);
        return Parser(tokens);
        /*QString num = text.section(' ',1,1);
        bool isValid = true;
        double res = eval(num, isValid);
        if(num.isEmpty() || !isValid) {
            Report("Syntax Error : [NUMBER] parameter missing");
            return false;
        }
        Wait(res);
        return Parser(text.section(' ',2));*/
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "TO")
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
        if(defaultNames.count(name))
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
            ProcTokens.push_back(vector<vector<Token>>());
        }
        else
        {
            Procs[reg_id - 1] = QStringList();
            ProcTokens[reg_id - 1] = vector<vector<Token>>();
        }
        Defmode = true;
        Def_name = name;
        Def_id = reg_id - 1;
        return true;
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::END) {

        return true;
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "REPEAT")
    {
        bool ok;
        double num = getNum(tokens, ok);
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
            case TokenType::LBRACKET: layer++;
            case TokenType::RBRACKET: layer--;
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

    if(word.type == TokenType::KEYWORD && (word.lexeme == "ST" || word.lexeme == "SHOWTURTLE"))
    {
        PArea->showTurtle();
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && (word.lexeme == "HT" || word.lexeme == "HIDETURTLE"))
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
        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        PArea->Forward(num);
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::BK)
    {
        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        PArea->Backward(num);
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::LT)
    {
        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        PArea->turnLeft(num);
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::RT)
    {
        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        PArea->turnRight(num);
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && keyConvert(word.lexeme) == Keywords::SETW)
    {
        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        PArea->setW(num);
        return Parser(tokens);
    }

    if(ProcNames.count(word.lexeme))
    {
        qDebug() << "!!!";
        rec_layers++;
        int id = ProcNames[word.lexeme];

        for(int i = 0;i < ProcTokens[id - 1].size();i++)
        {
            vector<Token> temp = ProcTokens[id - 1][i];
            if(!Parser(temp))
                return false;
        }
        rec_layers--;
        return Parser(tokens);
    }

    Report(word.lexeme + " is not a LOGO procedure.");
    return false;
}
