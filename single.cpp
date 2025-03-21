
#include "_cursor.h"
#include "paint.h"

using std::vector;

const double Xrange = 900.0, Yrange = 400.0;

int PaintArea::Turns_X(double pos)
{
    return ceil((abs(pos) - Xrange / 2.0) / Xrange) + 0.5;
}
int PaintArea::Turns_Y(double pos)
{
    return ceil((abs(pos) - Yrange / 2.0) / Yrange) + 0.5;
}

void PaintArea::turnLeft(double deg){
    cursor.Deg -= deg / 180.0 * PI;
    sync();
}

void PaintArea::turnRight(double deg){
    cursor.Deg += deg / 180.0 * PI;
    sync();
}

void PaintArea::clearScreen(){
    frame->clear();
    cursor.X = 0;
    cursor.Y = 0;
    cursor.Deg = -PI / 2.0;
    sync();
    frame->repaint();
}

void PaintArea::Home(){
    cursor.X = 0;
    cursor.Y = 0;
    cursor.Deg = -PI / 2.0;
    sync();
}

void PaintArea::showTurtle(){
    cursor.show();
}

void PaintArea::hideTurtle(){
    cursor.hide();
}

void PaintArea::penUp(){
    penD = false;
}

void PaintArea::penDown(){
    penD = true;
}

void PaintArea::setW(int width){
    frame->penw = width;
}

void PaintArea::setPC(QColor col){
    frame->penc = col;
}

void PaintArea::Forward(double dis){
    auto vec = cursor.Vector(dis);
    //int turns = std::max(Turns_X(cursor.X + vec.first), Turns_Y(cursor.Y + vec.second));
    vector<pair<double,int>> Ratios;//(ratio, type)
    Ratios.push_back(std::make_pair(1.0,0));
    double new_X = cursor.X + vec.first, new_Y = cursor.Y + vec.second;
    if(new_X < -Xrange / 2.0)
    {
        int t = Turns_X(new_X);
        for(int i = 0;i < t;i++) {
            Ratios.push_back(std::make_pair((-(2 * i + 1) * (Xrange / 2.0) - cursor.X) / (new_X - cursor.X), 0));
        }
    }
    if(new_X > Xrange / 2.0)
    {
        int t = Turns_X(new_X);
        for(int i = 0;i < t;i++) {
            Ratios.push_back(std::make_pair(((2 * i + 1) * (Xrange / 2.0) - cursor.X) / (new_X - cursor.X), 1));
        }
    }
    if(new_Y < -Yrange / 2.0)
    {
        int t = Turns_Y(new_Y);
        for(int i = 0;i < t;i++) {
            Ratios.push_back(std::make_pair((-(2 * i + 1) * (Yrange / 2.0) - cursor.Y) / (new_Y - cursor.Y), 2));
        }
    }
    if(new_Y > Yrange / 2.0)
    {
        int t = Turns_Y(new_Y);
        for(int i = 0;i < t;i++) {
            Ratios.push_back(std::make_pair(((2 * i + 1) * (Yrange / 2.0) - cursor.Y) / (new_Y - cursor.Y), 3));
        }
    }
    std::sort(Ratios.begin(),Ratios.end());
    double offset_X = 0.0, offset_Y = 0.0;
    for(int i = 0;i < (int)Ratios.size();i++)
    {
        double R = Ratios[i].first;
        int Type = Ratios[i].second;
        //qDebug()<<R<<' '<<Type;
        double curX = cursor.X + R * (new_X - cursor.X), curY = cursor.Y + R * (new_Y - cursor.Y);
        if(penD)
            frame->Line(curX - offset_X, curY - offset_Y);
        else
            frame->moveTo(curX - offset_X, curY - offset_Y);
        if(i + 1 == (int)Ratios.size())
            cursor.X = curX - offset_X, cursor.Y = curY - offset_Y, cursor.sync();
        switch(Type)
        {
            case 0:offset_X += -Xrange;break;
            case 1:offset_X += Xrange;break;
            case 2:offset_Y += -Yrange;break;
            case 3:offset_Y += Yrange;break;
        }
        if(i + 1 < (int)Ratios.size())
            frame->moveTo(curX - offset_X, curY - offset_Y);

    }
    frame->repaint();
}

void PaintArea::Backward(double dis){
    cursor.Deg += PI;
    PaintArea::Forward(dis);
    cursor.Deg -= PI;
    cursor.sync();
}

//void MainWindow::Forward(){

//}
