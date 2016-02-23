#ifndef _WORLD_H
#define _WORLD_H

#include <vector>
#include "mathHelper.h"
#include "object.h"

class World {
    std::vector<Object*> objectList;
    // attributes?
private:
    void addObject(Object *obj) {
        objectList.push_back(obj);
    }

    // Spawn will return the color we should use for the pixel in the ray
    Color spawn(Ray ray) {
        std::vector<Object*>::iterator it;
        std::vector<Point> vPoint;
        std::vector<Color> vColor;
        int i = 0;

        // we will go through the objects in the world and look for intersections
        for(it = objectList.begin() ; it < objectList.end() ; ++it, ++i) {
            vPoint.push_back( (*it)->intersect(ray) );
            vColor.push_back( (*it)->getColor() );
        }
        
        // check which intersection point is closer to ray origin
        float auxDist, minDist = -1;
        Color finalColor; 
        
        for(i = 0; i < objectList.size(); ++i) {
            auxDist = distance(ray.getOrigin(), vPoint[i]);

            if (auxDist != 0 && minDist == -1) {
                minDist = auxDist;
                finalColor = vColor[i];
            } else if (auxDist < minDist) {
                minDist = auxDist;
                finalColor = vColor[i];
            }
        }

        // if minDist still equal -1, return black (background)
        if (minDist == -1)
            return Color(0,0,0);

        return finalColor;
    }
    

};

#endif