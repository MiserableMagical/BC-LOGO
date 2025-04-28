/*
 * eval.cpp 表达式求值
 * beacon_cwk 25/02/19
*/

#include "QFileDialog"
#include "QFile"
#include "QLineEdit"
#include "QRegularExpression"
#include "mainwindow.h"
#include <stack>
#include <random>

#define Report setListenerText
#define RegExp QRegularExpression

std::unordered_map<QString, int> precedence = {
    {"=",1},
    {">",2},
    {"<",2},
    {">=",2},
    {"<=",2},
    {"|", 51},
    {"&", 52},
    {"+", 101},
    {"-", 101},
    {"*", 102},
    {"/", 102},
    {"%", 102},
    {"RANDOM", 1000},
    {"SIN",1000},
    {"COS",1000}
};

// 判断字符是否为运算符
bool isOperator(QString c) {
    return precedence.find(c) != precedence.end();
}

extern map<QString, Keywords> mathFunctions;

bool MainWindow::isFunction(QString name)
{
    return mathFunctions.count(name);
}

// 比较两个运算符的优先级
bool hasHigherPrecedence(QString op1, QString op2) {
    return precedence[op1] >= precedence[op2];
}

const qreal Epsilon = 1e-11;
bool Truth(qreal val)
{
    if(abs(val) < Epsilon) return 0;
    return 1;
}

std::mt19937 randgen(time(0));
int getRand(int lim)
{
    if(lim <= 1) return 1;
    return std::abs(int(randgen())) % lim + 1;
}

vector<qreal> MainWindow::eval(vector<Token> &expr, bool &isValid)
{
    isValid = true;
    std::stack<QString> stack;
    QString Num;
    std::vector<QString> post;

    bool needOP = false;
    for (auto c : expr) {
        if(c.type == TokenType::NUMBER || (c.type == TokenType::IDENTIFIER && isVariable(c.lexeme)))
        {
            if(needOP)
            {
                //弹出所有运算符
                while (!stack.empty()) {
                    post.push_back(stack.top());
                    stack.pop();
                }
            }
            if(c.type == TokenType::NUMBER) post.push_back(c.lexeme);
            else post.push_back(QString::number(getVariable(c.lexeme)));
            needOP = true;
        }
        else
        {
            if(c.type == TokenType::LPAREN) {
                stack.push("(");
            }
            else if(c.type == TokenType::RPAREN) {
                while(!stack.empty() && stack.top() != "(") {
                    post.push_back(stack.top());
                    stack.pop();
                }
                if(stack.empty())
                {
                    setListenerText("Invalid Expression : parentheses don't match");
                    isValid = false;
                    return {};
                }
                stack.pop();
            }
            else if(c.type == TokenType::OPERATOR || (c.type == TokenType::IDENTIFIER && isFunction(c.lexeme))) {
                while (!stack.empty() && stack.top() != "(" && hasHigherPrecedence(stack.top(), c.lexeme)) {
                    post.push_back(stack.top());
                    stack.pop();
                }
                stack.push(c.lexeme);
                if(!isFunction(c.lexeme)) needOP = false;
            }
            else {
                isValid = false;
                return {};
            }
        }
    }
    if(!Num.isEmpty()) {
        post.push_back(Num);
        Num = "";
    }

    // 弹出栈中剩余的运算符
    while (!stack.empty()) {
        post.push_back(stack.top());
        stack.pop();
    }

    std::stack<qreal> results;
    for(QString &cur : post)
    {
        if(precedence.count(cur)) /*cur == "+" || cur == "-" || cur == "*" || cur == "/"*/
        {
            if(results.size() == 1 && cur == "-") //按负号处理
            {
                qreal fir = results.top();
                results.pop();
                results.push(-fir);
                continue;
            }
            if(results.size() < 1)
            {
                setListenerText("Invalid Expression");
                isValid = false;
                return {};
            }
            if(cur == "SIN")
            {
                qreal top = results.top();
                results.pop();
                results.push(sin(top));
                continue;
            }
            if(cur == "COS")
            {
                qreal top = results.top();
                results.pop();
                results.push(cos(top));
                continue;
            }
            if(cur == "RANDOM")
            {
                qreal top = results.top();
                results.pop();
                results.push(getRand(top + Epsilon));
                continue;
            }
            if(results.size() < 2)
            {
                setListenerText("Invalid Expression");
                isValid = false;
                return {};
            }
            qreal sec = results.top();
            results.pop();
            qreal fir = results.top();
            results.pop();
            if(cur == "+")
                results.push(fir + sec);
            if(cur == "-")
                results.push(fir - sec);
            if(cur == "*")
                results.push(fir * sec);
            if(cur == "/")
                results.push(fir / sec);
            if(cur == "%")
            {
                if((long long)sec == 0) results.push(0.0 / 0.0);
                else results.push((long long)fir % (long long)sec);
            }
            if(cur == "&")
                results.push(Truth(fir) && Truth(sec));
            if(cur == "|")
                results.push(Truth(fir) || Truth(sec));
            if(cur == ">")
                results.push(fir > sec);
            if(cur == "<")
                results.push(fir < sec);
            if(cur == ">=")
                results.push(fir >= sec);
            if(cur == "<=")
                results.push(fir <= sec);
            if(cur == "=")
                results.push(abs(fir - sec) < Epsilon);
        }
        else
        {
            results.push(cur.toDouble());
        }
    }

    vector<qreal> retVal;
    while(!results.empty())
    {
        retVal.push_back(results.top());
        results.pop();
    }
    reverse(retVal.begin(), retVal.end());
    return retVal;
}


//找到该表达式的范围
void MainWindow::getExpBound(std::vector<Token> & tokens,std::vector<Token> & exp,int &result)
{

    //bool nonEmpty = false;
    while(!tokens.empty() && ((tokens.back().type == TokenType::NUMBER
                                || tokens.back().type == TokenType::LPAREN
                                || tokens.back().type == TokenType::RPAREN
                                || tokens.back().type == TokenType::OPERATOR
                                || tokens.back().type == TokenType::IDENTIFIER)))
    {
        //nonEmpty = true;
        if(tokens.back().type == TokenType::IDENTIFIER) {// check if it's a variable
            if(!isVariable(tokens.back().lexeme) && !isFunction(tokens.back().lexeme)) break;
        }
        exp.push_back(tokens.back());
        tokens.pop_back();
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
    if(ok) {
        if(res.empty())
        {
            ok = false;
            return 0;
        }
        return res.front();
    }
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
