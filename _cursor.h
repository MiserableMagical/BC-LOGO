#include<QLabel>

#ifndef _CURSOR_H
#define _CURSOR_H

using std::pair;

const double PI = acos(-1.0);

class _Cursor
{
public:
    _Cursor(){}
    double X = 0, Y = 0;
    double Deg = -PI / 2.0;//up
    QLabel *img;

    void show()
    {
        img->show();
    }
    void hide()
    {
        img->hide();
    }
    pair<double,double> Vector(double dis){
        return std::make_pair(dis * cos(Deg), dis * sin(Deg));
    }
    QPixmap rotation(const QPixmap img, double deg){
        QTransform transform;
        transform.rotate(deg); // 旋转角度为45度
        return img.transformed(transform);
    }
    void sync(){
        img->move(X + 485,Y + 235);
        img->setPixmap(rotation(QPixmap(":/new/cursor/CURSOR.jpg"), (Deg + PI / 2.0) / PI * 180.0));
    }
};

#endif // _CURSOR_H
