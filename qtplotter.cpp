#include "qtplotter.h"

#include "paint.h"

#include <QColor>

QtPlotter::QtPlotter(PaintArea *area)
    : area_(area)
{
}

void QtPlotter::forward(double distance)
{
    area_->Forward(distance);
}

void QtPlotter::backward(double distance)
{
    area_->Backward(distance);
}

void QtPlotter::turnLeft(double degrees)
{
    area_->turnLeft(degrees);
}

void QtPlotter::turnRight(double degrees)
{
    area_->turnRight(degrees);
}

void QtPlotter::clearScreen()
{
    area_->clearScreen();
}

void QtPlotter::clean()
{
    area_->Clean();
}

void QtPlotter::home()
{
    area_->Home();
}

void QtPlotter::showTurtle()
{
    area_->showTurtle();
}

void QtPlotter::hideTurtle()
{
    area_->hideTurtle();
}

void QtPlotter::penUp()
{
    area_->penUp();
}

void QtPlotter::penDown()
{
    area_->penDown();
}

void QtPlotter::setPenWidth(double width)
{
    area_->setW(static_cast<int>(width));
}

void QtPlotter::setPenColor(const QString &color)
{
    area_->setPC(QColor(color));
}

void QtPlotter::setPenColorRgb(int red, int green, int blue)
{
    area_->setPC(QColor(red, green, blue));
}

void QtPlotter::setBackground(const QString &color)
{
    area_->setBG(QColor(color));
}

void QtPlotter::setX(double x)
{
    area_->setX(x);
}

void QtPlotter::setY(double y)
{
    area_->setY(y);
}

void QtPlotter::setXY(double x, double y)
{
    area_->setXY(x, y);
}
