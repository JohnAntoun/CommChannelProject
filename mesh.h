#ifndef ZONE_H
#define ZONE_H

#include <QWidget>
#include <QBrush>

class Mesh : public QRectF
{
public:
    Mesh(qreal x, qreal y, qreal width, qreal height, int meshType);
    qreal centerX, centerY, w, h, antennaH;
    int mesh;
    QBrush brush;

private:
    void setBrush( int mesh );
};

#endif // ZONE_H
