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

qreal MainWindow::eval(QString &expr, bool &isValid)
{
    isValid = true;
    std::stack<QChar> stack;
    QString Num;
    std::vector<QString> post;

    for (QChar c : expr) {
        // 如果是操作数，直接添加到后缀表达式
        if (('0' <= c && c <= '9') || (!Num.isEmpty() && c == '.')) {
            Num += c;
        }
        // 如果是左括号，压入栈
        else
        {
            if(!Num.isEmpty()) {
                //std::reverse(Num.begin(),Num.end());
                post.push_back(Num);
                Num = "";
            }
            if(c == '(') {
                stack.push(c);
            }
            // 如果是右括号，弹出栈中的元素直到遇到左括号
            else if (c == ')') {
                while (!stack.empty() && stack.top() != '(') {
                    post.push_back(stack.top());
                    stack.pop();
                }
                if(stack.empty())
                {
                    setListenerText("Invalid Expression : parentheses don't match");
                    isValid = false;
                    return 0;
                }
                stack.pop();  // 弹出左括号
            }
            // 如果是运算符
            else if (isOperator(c)) {
                while (!stack.empty() && stack.top() != '(' && hasHigherPrecedence(stack.top(), c)) {
                    post.push_back(stack.top());
                    stack.pop();
                }
                stack.push(c);
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
