#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QGraphicsScene>
#include<QGraphicsItem>
#include<QGraphicsView>
#include<QDebug>
#include<QPen>
#include"mesh.h"
#include "ray.h"
#include "wall.h"
#include <array>
#include<complex>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QGraphicsScene *scene;

private slots:
    void makePathLossPlots(QVector<qreal> dist_vec, QVector<qreal> Prx_vec);
    void makeCellRangePlot(qreal slope, qreal intercept, qreal sigma);

private:
    Ui::MainWindow *ui;
      //qreal imageMethodColorMap(Mesh TX, Mesh RX, QList<Wall> walls, qreal scaleX, qreal scaleY);
      //qreal imageMethod2(Mesh TX, Mesh RX, QList<Wall> walls, qreal scaleX, qreal scaleY);
      complex<qreal> imageMethod5G(Mesh TX, Mesh RX, QList<Wall> walls, QList<Wall> diffr,  qreal scaleX, qreal scaleY);
      complex<qreal> imageMethod5GColorMap(Mesh TX, Mesh RX, QList<Wall> walls, QList<Wall> diffr);

      void draw_ray(QPointF A, QPointF B, int n_refl, qreal scaleX, qreal scaleY);
      qreal reflexion_coef(QPointF A, QPointF B, Wall wall);
      qreal transmission_coef(QPointF A, QPointF B, QList<Wall> walls);


      QPointF mirrorPoint(QPointF P, QLineF d);
      complex<qreal> phase(qreal beta, qreal d);
};

#endif // MAINWINDOW_H
