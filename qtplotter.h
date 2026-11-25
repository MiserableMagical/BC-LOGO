#pragma once

#include "core/runtime.h"

class PaintArea;

/// 把 core 的 Plotter 接口接到现有的 PaintArea。
class QtPlotter final : public logo::Plotter {
public:
    explicit QtPlotter(PaintArea *area);

    void forward(double distance) override;
    void backward(double distance) override;
    void turnLeft(double degrees) override;
    void turnRight(double degrees) override;
    void clearScreen() override;
    void clean() override;
    void home() override;
    void showTurtle() override;
    void hideTurtle() override;
    void penUp() override;
    void penDown() override;
    void setPenWidth(double width) override;
    void setPenColor(const QString &color) override;
    void setPenColorRgb(int red, int green, int blue) override;
    void setBackground(const QString &color) override;
    void setX(double x) override;
    void setY(double y) override;
    void setXY(double x, double y) override;

private:
    PaintArea *area_;
};
