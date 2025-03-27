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
    {"IF",Keywords::IF}
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
    //reverse(exp.begin(),exp.end());
    //for(auto c : exp) qDebug() << "!" << c.lexeme;
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
    double res = eval(exp, ok);
    if(ok) return res;
    return 0;
}


/*以下为关键字读入处理部分*/

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

bool MainWindow::dealsetW(std::vector<Token> & tokens)
{
    bool ok;
    double num = getNum(tokens, ok);
    if(ok == false)
        return false;
    PArea->setW(num);
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

/*以上为关键字读入处理部分*/

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

    if(word.type == TokenType::KEYWORD && word.lexeme == "PRINT") {
        if(!dealPrint(tokens)) return false;
        return Parser(tokens);
    }

    if(word.type == TokenType::KEYWORD && word.lexeme == "MAKE")
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
        if(defaultNames.count(name) || ProcNames.count(name))
        {
            Report(name + " is already in use.Try a different name.");
            return false;
        }

        bool ok;
        double num = getNum(tokens, ok);
        if(ok == false)
            return false;
        qDebug() << name << ' ' << num;
        varNames[name] = num;
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
        num = floor(num + 1e-6);
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
