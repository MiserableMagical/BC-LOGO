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

set<QString> defaultNames = {"FD","FORWARD","BK","BACKWARD","LT","LEFT","RT","RIGHT",
                             "CS","ST","SHOWTURTLE","HT","HIDETURTLE","PU","PENUP","PD","PENDOWN","REPEAT","TO","END","HOME","SETW"};

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

bool MainWindow::Parser(QString text)
{
    if(text.length() == 0)
        return true;
    if(rec_layers > 100)
    {
        Report("Too many recursive calls");
        return false;
    }

    text = text.trimmed();
    QString word = text.section(' ',0,0);
    word = word.toUpper();

    if(word == "WAIT")
    {
        QString num = text.section(' ',1,1);
        bool isValid = true;
        double res = eval(num, isValid);
        if(num.isEmpty() || !isValid) {
            Report("Syntax Error : [NUMBER] parameter missing");
            return false;
        }
        Wait(res);
        return Parser(text.section(' ',2));
    }

    if(word == "TO")
    {
        QString name = text.section(' ',1,1);
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
            Procs.push_back(QStringList());
        else
            Procs[reg_id - 1] = QStringList();
        Defmode = true;
        Def_name = name;
        Def_id = reg_id - 1;
        return true;
    }

    if(word == "REPEAT")
    {
        static RegExp pat("[ \[]");
        QString num = text.section(pat,1,1);
        bool isValid = true;
        double res = eval(num, isValid);
        if(num.isEmpty() || !isValid)
        {
            Report("Syntax Error : [NUMBER] parameter missing");
            return false;
        }
        //QString remains = text.section(pat, 2);
        if(!checkBrackets(text))
        {
            Report("Syntax Error : Parentheses don't match");
            return false;
        }
        int fir_pos = -1, lst_pos = -1;
        for(int i = 0;i < text.size();i++)
        {
            if(text[i] == '[')
            {
                fir_pos = i;
                int layers = 0;
                for(int j = i;j < text.size();j++)
                {
                    if(text[j] == '[') layers++;
                    if(text[j] == ']') layers--;
                    if(layers == 0) {
                        lst_pos = j;
                        break;
                    }
                }
                break;
            }
        }
        for(int i = 1;i <= res;i++) {
            if(!Parser(text.mid(fir_pos + 1, lst_pos - fir_pos - 1)))
                return false;
        }
        return Parser(text.mid(lst_pos + 1));
    }

    if(word == "CS") {
        PArea->clearScreen();
        return Parser(text.section(' ',1));
    }

    if(word == "HOME") {
        PArea->Home();
        return Parser(text.section(' ',1));
    }

    if(word == "ST" || word == "SHOWTURTLE")
    {
        PArea->showTurtle();
        return Parser(text.section(' ',1));
    }

    if(word == "HT" || word == "HIDETURTLE")
    {
        PArea->hideTurtle();
        return Parser(text.section(' ',1));
    }

    if(word == "PU" || word == "PENUP")
    {
        PArea->penUp();
        return Parser(text.section(' ',1));
    }

    if(word == "PD" || word == "PENDOWN")
    {
        PArea->penDown();
        return Parser(text.section(' ',1));
    }

    if(word == "FD" || word == "FORWARD")
    {
        QString num = text.section(' ',1,1);
        bool isValid = true;
        double res = eval(num, isValid);
        if(num.isEmpty() || !isValid)
        {
            Report("Syntax Error : [NUMBER] parameter missing");
            return false;
        }
        PArea->Forward(res);
        return Parser(text.section(' ',2));
    }

    if(word == "BK" || word == "BACKWARD")
    {
        QString num = text.section(' ',1,1);
        bool isValid = true;
        double res = eval(num, isValid);
        if(num.isEmpty() || !isValid)
        {
            Report("Syntax Error : [NUMBER] parameter missing");
            return false;
        }
        PArea->Backward(res);
        return Parser(text.section(' ',2));
    }

    if(word == "LT" || word == "LEFT")
    {
        QString num = text.section(' ',1,1);
        bool isValid = true;
        double res = eval(num, isValid);
        if(num.isEmpty() || !isValid)
        {
            Report("Syntax Error : [NUMBER] parameter missing");
            return false;
        }
        PArea->turnLeft(res);
        return Parser(text.section(' ',2));
    }

    if(word == "RT" || word == "RIGHT")
    {
        QString num = text.section(' ',1,1);
        bool isValid = true;
        double res = eval(num, isValid);
        if(num.isEmpty() || !isValid)
        {
            Report("Syntax Error : [NUMBER] parameter missing");
            return false;
        }
        PArea->turnRight(res);
        return Parser(text.section(' ',2));
    }

    if(word == "SETW")
    {
        QString num = text.section(' ',1,1);
        bool isValid = true;
        double res = eval(num, isValid);
        if(num.isEmpty() || !isValid)
        {
            Report("Syntax Error : [NUMBER] parameter missing");
            return false;
        }
        PArea->setW(res);
        return Parser(text.section(' ',2));
    }

    if(ProcNames.count(word))
    {
        rec_layers++;
        int id = ProcNames[word];
        for(int i = 0;i < Procs[id - 1].size();i++)
        {
            if(!Parser(Procs[id - 1][i]))
                return false;
        }
        rec_layers--;
        return Parser(text.section(' ',1));
    }

    Report(word + " is not a LOGO procedure.");
    return false;
}
