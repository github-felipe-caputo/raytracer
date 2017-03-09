#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "mathHelper.h"
#include "object.h"

void translate (Object *obj, double tx, double ty, double tz) {
    std::vector<Point> vertices = obj->getPoints();
    std::vector<Point> result;
    
    for(std::vector<Point>::iterator it = vertices.begin() ; it < vertices.end() ; ++it) {
        result.push_back( Point(it->x + tx, it->y + ty, it->z + tz) );
    }

    obj->setPoints(result);
}


#endif
