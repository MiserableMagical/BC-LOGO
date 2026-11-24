#pragma once

#include <QString>

namespace logo {

/// 控制台/听者区。测试用字符串收集器，Qt 层用 myListener 适配。
class Console {
public:
    virtual ~Console() = default;
    virtual void print(const QString &text) = 0;
};

/// 绘图/海龟状态接口。core 不依赖 QColor/QPainter；
/// Qt 层用 PaintArea 适配，测试用 RecordingPlotter。
class Plotter {
public:
    virtual ~Plotter() = default;

    virtual void forward(double) {}
    virtual void backward(double) {}
    virtual void turnLeft(double) {}
    virtual void turnRight(double) {}
    virtual void clearScreen() {}
    virtual void clean() {}
    virtual void home() {}
    virtual void showTurtle() {}
    virtual void hideTurtle() {}
    virtual void penUp() {}
    virtual void penDown() {}
    virtual void setPenWidth(double) {}
    virtual void setPenColor(const QString &) {}
    virtual void setPenColorRgb(int, int, int) {}
    virtual void setBackground(const QString &) {}
    virtual void setX(double) {}
    virtual void setY(double) {}
    virtual void setXY(double, double) {}
};

/// 纯逻辑运行（CLI/测试）时的空绘图器。
class NullPlotter final : public Plotter {
};

} // namespace logo
