#ifndef WALL_H
#define WALL_H

#include <QWidget>
#include<QPen>
#include<QPointF>

class Wall : public QLineF
{
public:

    qreal conductivity, permittivity, thickness;
    QPen outlinePen;

    Wall(qreal x1, qreal y1, qreal x2, qreal y2, qreal thickness, int wallType);
};
#endif // WALL_H
