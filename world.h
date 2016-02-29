#ifndef _WORLD_H
#define _WORLD_H

#include <vector>
#include "mathHelper.h"
#include "object.h"
#include "lightSource.h"
#include "illuminationModel.h"

class World {
    // List of objects in this world
    std::vector<Object*> objectList;

    // List of lights in this world
    std::vector<LightSource> lightList;

    // the world might have a illumination model
    IlluminationModel *illuminationModel;

    // Attribute for phong illumination
    Color ambientLight;
    bool phong;

public:

    World () {
        // Every illumination model is set off by default
        phong = false;
    }

    void setUpPhongIllumination(Color amLight) {
        Phong ph;
        illuminationModel = &ph;
        ambientLight = amLight;
        phong = true;
    }

    void addObject(Object *obj) {
        objectList.push_back(obj);
    }

    void addLight(LightSource light) {
        lightList.push_back(light);
    }

    // Spawn will return the color we should use for the pixel in the ray
    Color spawn(Ray ray) {
        std::vector<Object*>::iterator it;
        std::vector<Point> vPoint;
        std::vector<Color> vColor;
        unsigned int i = 0;

        // we will go through the objects in the world and look for intersections
        for(it = objectList.begin() ; it < objectList.end() ; ++it, ++i) {
            vPoint.push_back( (*it)->intersect(ray) );
            vColor.push_back( (*it)->getColor() );
        }
        
        /* This part of the code is goddamn DISGUSTING and I need to think of a better way to go about it */

        // check which intersection point is closer to ray origin
        float auxDist, minDist = -1;
        Color finalColor; 
        
        for(i = 0; i < objectList.size(); ++i) {
            auxDist = distance(ray.getOrigin(), vPoint[i]);

            if (auxDist != 0 && minDist == -1) {
                minDist = auxDist;
                finalColor = vColor[i];
            } else if (auxDist != 0 && auxDist < minDist) {
                minDist = auxDist;
                finalColor = vColor[i];
            }
        }

        // if minDist still equal -1, return black (background)
        if (minDist == -1)
            return Color(0,0,0);

        //
        // having the point here is prob where I should check if from that point I reach the light position,
        // If I do, start the illumination stuff
        //
        //
        //if(phong) {
        //    // do stuff
        //}

        return finalColor;
    }
    

};

#endif