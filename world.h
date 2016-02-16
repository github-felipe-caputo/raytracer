#ifndef _WORLD_H
#define _WORLD_H

#include <vector>
#include "mathHelper.h"
#include "object.h"

using namespace std;

class World {
    vector<Object*> objectList;
    // attributes?
private:
    void addObject(Object *obj) {
        objectList.push_back(obj);
    }

    // Spawn will return the color we should use for the pixel in the ray
    /*
    Color spawn(Ray ray) {
        Point aux[objectList.size()];
        Color auxColor[objectList.size()];

        // we will go through the objects in the world and look for intersections
        for(i = 0, it=objectList.begin() ; it < objectList.end(); it++, i++) {
            aux[i] = *it.interesect(Ray);
            auxColor[i] = *it.getColor();
        }

        // check which intersection point is closer to ray origin
        float minDist = -1;

        for(i = 0; i < objectList.size(); ++i) {

        }
    }
    */

};

#endif