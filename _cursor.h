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
    int Size = 10;
    const int baseSize = 10;
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
        transform.rotate(deg);
        double val = std::max(abs(sin(deg / 180.0 * PI)), abs(cos(deg / 180.0 * PI)));
        int nw_Size = int(baseSize / val);
        //if(val < 0.88) nw_Size = 12;//adjusted size
        //else nw_Size = 10;
        Size = nw_Size;
        return img.transformed(transform);
    }
    void sync(){
        img->setPixmap(rotation(QPixmap(":/new/cursor/CURSOR.jpg"), (Deg + PI / 2.0) / PI * 180.0));
        img->move(X + 490 - Size / 2,Y + 240 - Size / 2);
        img->resize(Size, Size);
    }
};

#endif // _CURSOR_H
