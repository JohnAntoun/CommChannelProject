#ifndef MAPCREATION_H
#define MAPCREATION_H

#include<QtWidgets>
#include"wall.h"

std::tuple< QList<Wall>, qreal, qreal> createStudioMap(qreal mapWidth, qreal mapHeigth);
std::tuple< QList<Wall>, QList<Wall>,qreal, qreal> createStreetMap(qreal mapWidth, qreal mapHeigth);



#endif // MAPCREATION_H
