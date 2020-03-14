#include "ray.h"

Ray::Ray( int reflectionsNumber) : reflections(reflectionsNumber)
{
   setOutlinePen(reflections);
}

void Ray :: setOutlinePen(int r){
    switch(r){

    case 0 : outlinePen.setColor(Qt::darkRed); break;
    case 1 : outlinePen.setColor(Qt::red); break;
    case 2 : outlinePen.setColor(Qt::white); break;
    case 3 : outlinePen.setColor(Qt::white); break;

    }

}
