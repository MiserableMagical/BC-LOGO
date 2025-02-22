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
public : QPixmap *pixmap = new QPixmap(900, 400);
    double x,y;
    int penw = 1;
    public : void paintEvent(QPaintEvent * event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(0,0,*pixmap);
    }
    void Line(double x1,double y1)
    {
        QPainter painter(pixmap);
        QPen pen;
        QBrush brush;
        painter.translate(450,200);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        brush.setColor(Qt::white);
        pen.setColor(Qt::black);
        pen.setWidth(penw);
        painter.setBrush(brush);
        painter.setPen(pen);
        QPainterPath path;
        path.moveTo(x,y);
        path.lineTo(x1,y1);
        painter.drawPath(path);
        //painter.drawLine(x,y,x1,y1);
        x = x1;y = y1;
    }
    void moveTo(double x1,double y1)
    {
        x = x1;
        y = y1;
    }
    void clear()
    {
        pixmap->fill(Qt::transparent);
    }
};

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
        frame2->resize(902,402);
        frame->move(40,40);
        frame->resize(900,400);
        frame->pixmap->fill(Qt::transparent);
        frame->raise();
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
    void clearScreen();
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
};
