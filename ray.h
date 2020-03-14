#ifndef RAY_H
#define RAY_H

#include <QWidget>
#include<QPen>

class   Ray : public QLineF
{
public:
    Ray( int reflectionsNumber);
    int reflections;

    QPen outlinePen;
    QVector<QPointF> reflectionPointsList;
    QVector<QPointF> transmitionPointsList;
    QPointF lastMirror;

    void setLastMirror(QPointF p);

private:
    void setOutlinePen( int r );


};

#endif // RAY_H
