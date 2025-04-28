#include "mainwindow.h"
#include "QFileDialog"
#include "ui_mainwindow.h"
#include "QFile"
#include "QLineEdit"
#include "_cursor.h"
#include "QPainter"
#include "QPainterPath"

#ifndef PAINT_H
#define PAINT_H

#include "qmainwindow.h"
#endif // PAINT_H

//QFrame_是画图区域本身，用于处理图形绘制
const int paintHeight = 400, paintWidth = 900;
class QFrame_ : public QFrame{
    /*public : QPainterPath path;
    public : void paintEvent(QPaintEvent * event)
    {
        QPainter painter(this);
        QPen pen;
        QBrush brush;
        painter.translate(450,200);
        painter.setRenderHint(QPainter::Antialiasing);
        brush.setColor(Qt::white);
        pen.setWidth(1);
        painter.setBrush(brush);
        painter.setPen(pen);
        //painter.drawRect(0,0,900,400);
        setPalette(QPalette(Qt::white));
        painter.drawPath(path);
    }*/
public : QPixmap *pixmap = new QPixmap(paintWidth, paintHeight);
    double x,y;//当前位置
    int penw = 1;
    QColor penc = Qt::black;
    public : void paintEvent(QPaintEvent * event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(0,0,*pixmap);
    }
    //Line : 移到下一个位置并连线
    void Line(double x1,double y1)
    {
        QPainter painter(pixmap);
        QPen pen;
        QBrush brush;
        painter.translate(paintWidth / 2, paintHeight / 2);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        brush.setColor(Qt::white);
        pen.setColor(penc);
        pen.setWidth(penw);
        painter.setBrush(brush);
        painter.setPen(pen);
        QPainterPath path;
        path.moveTo(x,y);
        path.lineTo(x1,y1);
        painter.drawPath(path);
        x = x1;y = y1;
    }
    void moveTo(double x1,double y1)
    {
        x = x1;
        y = y1;
    }
    void clear()
    {
        pixmap->fill(Qt::white);
    }
};

//PaintArea集成了画图区域和光标，处理较复杂的操作
class PaintArea{
    _Cursor cursor;
    QFrame_ *frame;
    QFrame *frame2;
    bool penD = true;


    int Turns_X(double pos);
    int Turns_Y(double pos);

    public : void init(QMainWindow * pa)
    {
        frame = new QFrame_;
        frame->setParent(pa);
        frame2 = new QFrame;
        frame2->setParent(pa);
        frame2->setFrameShape(QFrame::StyledPanel);
        frame2->setFrameShadow(QFrame::Raised);
        frame2->move(39,39);
        frame2->resize(paintWidth + 2, paintHeight + 2);
        frame->move(40,40);
        frame->resize(paintWidth, paintHeight);
        frame->pixmap->fill(Qt::white);
        frame->raise();
        frame->x = frame->y = 0;
        cursor.img = new QLabel;
        cursor.img->setParent(pa);
        cursor.img->setPixmap(QPixmap(":/new/cursor/CURSOR.jpg"));
        cursor.img->resize(10,10);
        cursor.img->setScaledContents(true);
        cursor.img->show();
        cursor.sync();
    }
    void Forward(double dis);
    void Backward(double dis);
    void turnLeft(double deg);
    void turnRight(double deg);
    void setW(int width);
    void setPC(QColor);
    void setX(qreal);
    void setY(qreal);
    void setXY(qreal,qreal);
    void setBG(QColor);
    void clearScreen();
    void Clean();
    void showTurtle();
    void hideTurtle();
    void penUp();
    void penDown();
    void Home();
    void sync()
    {
        frame->x = cursor.X;
        frame->y = cursor.Y;
        cursor.sync();
    }
    QPixmap * getPixmap()
    {
        return frame->pixmap;
    }
};
