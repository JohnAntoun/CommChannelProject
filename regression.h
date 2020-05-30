#ifndef REGRESSION_H
#define REGRESSION_H

#include<QtWidgets>
#include <complex>
using namespace std;

qreal getSlope(QVector<qreal> &x, QVector<qreal> &y);
qreal getIntercept(QVector<qreal> &x, QVector<qreal> &y, qreal slope);
qreal getCost(QVector<qreal> &x, QVector<qreal> &y, qreal a, qreal b, qreal &da, qreal &db);
void linearRegression(QVector<qreal> &x, QVector<qreal> &y, qreal slope, qreal intercept);


#endif // REGRESSION_H
