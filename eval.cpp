#include "QFileDialog"
#include "QFile"
#include "QLineEdit"
#include "QRegularExpression"
#include "mainwindow.h"
#include <stack>

std::unordered_map<QChar, int> precedence = {
    {'+', 1},
    {'-', 1},
    {'*', 2},
    {'/', 2},
};

// 判断字符是否为运算符
bool isOperator(QChar c) {
    return precedence.find(c) != precedence.end();
}

// 比较两个运算符的优先级
bool hasHigherPrecedence(QChar op1, QChar op2) {
    return precedence[op1] >= precedence[op2];
}

qreal MainWindow::eval(vector<Token> &expr, bool &isValid)
{
    isValid = true;
    std::stack<QChar> stack;
    QString Num;
    std::vector<QString> post;

    for (auto c : expr) {
        if(c.type == TokenType::NUMBER)
        {
            post.push_back(c.lexeme);
        }
        else if(c.type == TokenType::IDENTIFIER && varNames.count(c.lexeme))
        {
            post.push_back(QString::number(varNames[c.lexeme]));
        }
        else
        {
            if(c.type == TokenType::LPAREN) {
                stack.push('(');
            }
            else if(c.type == TokenType::RPAREN) {
                while(!stack.empty() && stack.top() != '(') {
                    post.push_back(stack.top());
                    stack.pop();
                }
                if(stack.empty())
                {
                    setListenerText("Invalid Expression : parentheses don't match");
                    isValid = false;
                    return 0;
                }
                stack.pop();
            }
            else if(c.type == TokenType::OPERATOR) {
                while (!stack.empty() && stack.top() != '(' && hasHigherPrecedence(stack.top(), c.lexeme[0])) {
                    post.push_back(stack.top());
                    stack.pop();
                }
                stack.push(c.lexeme[0]);
            }
            else {
                isValid = false;
                return 0;
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
        if(cur == "+" || cur == "-" || cur == "*" || cur == "/")
        {
            if(results.size() < 2)
            {
                setListenerText("Invalid Expression : missing parameters");
                isValid = false;
                return 0;
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
        }
        else
        {
            results.push(cur.toDouble());
        }
    }

    return results.top();
}
