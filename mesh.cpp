#include "mesh.h"
#include <QDebug>

Mesh::Mesh(qreal x, qreal y, qreal width, qreal height, int meshType) :
    centerX(x), centerY(y), w(width), h(height), mesh(meshType)
{

    this->setX(centerX-width/2); this->setY(centerY-height/2); // QRectF defines the rectangle with top left corner
    this->setWidth(w); this->setHeight(h);
    this->brush.setStyle(Qt::SolidPattern);
    setBrush(mesh);

    this ->antennaH = 170; // let's say antennas height (both rx and tx) are at same height.
}

void Mesh :: setBrush(int m){
    // Setting up the brush color.
    // meshType value of 0 is for transmitter type of mesh, color red
    // meshType value of 1 is for receiver type of mesh, color green
    // TO DO -- still have to figure something out for color maping

    switch (m) {

    case 0:  this->brush.setColor(QColor(250,0,0,255));
            break;
    case 1: this->brush.setColor(QColor(0,250,0,255)); break;

    }
}

