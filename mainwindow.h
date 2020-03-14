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

private:
    Ui::MainWindow *ui;
      qreal imageMethodColorMap(Mesh TX, Mesh RX, QList<Wall> walls);
      qreal imageMethod2(Mesh TX, Mesh RX, QList<Wall> walls);
      qreal imageMethod5G(Mesh TX, Mesh RX, QList<Wall> walls);

      void draw_ray(QPointF A, QPointF B, int n_refl);
      qreal reflexion_coef(QPointF A, QPointF B, Wall wall);
      qreal transmission_coef(QPointF A, QPointF B, QList<Wall> walls);


      QPointF mirrorPoint(QPointF P, QLineF d);


};

#endif // MAINWINDOW_H
