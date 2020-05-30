#include "regression.h"


qreal getSlope(QVector<qreal> &x, QVector<qreal> &y){
    qreal sx = accumulate(x.begin(), x.end(), 0);
    qreal sy = accumulate(y.begin(), y.end(), 0);
    qreal sxx = inner_product(x.begin(), x.end(), x.begin(), 0);
    qreal sxy = inner_product(x.begin(), x.end(), y.begin(), 0);
    int n = static_cast<int>(x.size());
    // (n*sxy — sx*sy)/(n*sxx — sx*sx)
    qreal nor = n*sxy - sx*sy;
    qreal denor = n*sxx - sx*sx;
        if(denor!=0)
        {
            return nor/denor;
        }
    return numeric_limits<qreal>::max();
}

qreal getIntercept(QVector<qreal> &x, QVector<qreal> &y, qreal slope){
    qreal sx = accumulate(x.begin(), x.end(), 0);
    qreal sy = accumulate(y.begin(), y.end(), 0);
    int n = static_cast<int>(x.size());
    return (sy-slope*sx)/n;
}


// slope:a
// intercept:b
// derivative of slope: da
// derivative of intercept: db
qreal getCost(QVector<qreal> &x, QVector<qreal> &y, qreal a, qreal b, qreal &da, qreal &db){
    int n = static_cast<int>(x.size());
    qreal sx = accumulate(x.begin(), x.end(), 0);
    qreal sy = accumulate(y.begin(), y.end(), 0);
    qreal sxx = inner_product(x.begin(), x.end(), x.begin(), 0);
    qreal sxy = inner_product(x.begin(), x.end(), y.begin(), 0);
    qreal syy = inner_product(y.begin(), y.end(), y.begin(), 0);
    qreal cost = syy - 2*a*sxy - 2*b*sy + a*a*sxx + 2*a*b*sx + n*b*b;
    cost /= n;
    da = 2*(-sxy + a*sxx + b*sx)/n;
    db = 2*(-sy + a*sx + n*b)/n;
    return cost;
}

void linearRegression(QVector<qreal> &x, QVector<qreal> &y, qreal slope = 1, qreal intercept = 0){
    qreal lrate = 0.0002;
    qreal threshold = 0.0001;
    int iter = 0;
    while(true){
        qreal da = 0;
        qreal db = 0;
        qreal cost = getCost(x, y, slope, intercept, da, db);
        if(iter%1000==0){
            qDebug()<<"Iter: "<< iter << " cost = "<< cost << "da = " << da << " db = " << db << endl;
        }
        iter++;
        if(abs(da) < threshold && abs(db) < threshold){
            qDebug()<<"y = "<< slope << " * x + " << intercept << endl;
            break;
        }
        slope -= lrate* da;
        intercept -= lrate * db;
    }
}

// //Use exemple
//int main() {
//    QVector<qreal> x = { 71, 73, 64, 65, 61, 70, 65, 72, 63, 67, 64};
//    QVector<qreal> y = {160, 183, 154, 168, 159, 180, 145, 210, 132, 168, 141};

//    // initialize with random two points
//    qDebug()<< "Initialization with random 2 points"<<endl;
//    QVector<qreal> xSub = { 71, 73};
//    QVector<qreal> ySub = {160, 183};
//    qreal slopeSub =getSlope(xSub,ySub);
//    qreal interceptSub = getIntercept(xSub, ySub, slopeSub);
//    qDebug()<<"y = "<<slopeSub<<" * x + "<<interceptSub<<endl;

//    linearRegression(x, y, slopeSub, interceptSub);
//    qDebug()<< "Compare with ground truth"<<endl;

//    qreal slope = getSlope(x,y);
//    qreal intercept = getIntercept(x, y, slope);
//    qDebug()<<"y = "<<slope<<" * x + "<<intercept<<endl;
//    return 0;
//}
