#ifndef _WORLD_H
#define _WORLD_H

#include <vector>
#include <algorithm>
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
    bool phongIllumination;

    // Attribute for phong illumination
    Color backgroundRadiance;

public:

    World () {
        // Illumination set off by default
        phongIllumination = false;
    }

    void setUpPhongIllumination(Color amLight) {
        phongIllumination = true;
        backgroundRadiance = amLight;
    }

    void addObject(Object *obj) {
        objectList.push_back(obj);
    }

    void addLight(LightSource light) {
        lightList.push_back(light);
    }

    // Spawn will return the color we should use for the pixel in the ray
    Color spawn(Ray ray) {
        Point originRay = ray.getOrigin();
        Point intersection;

        std::vector<Object*>::iterator it;
        std::vector<LightSource>::iterator it2;
        std::vector<Point> vPoint;
        std::vector<float> vDist;
        unsigned int i = 0;

        // we will go through the objects in the world and look for intersections
        for(it = objectList.begin() ; it < objectList.end() ; ++it, ++i) {
            intersection = (*it)->intersect(ray);
            vPoint.push_back( intersection );
            vDist.push_back( distance(originRay, intersection) );
        }
        
        // objHit = indexMinElement(vDist)
        int objHit(indexMinElement(vDist));

        if( phongIllumination ) 
        {
            if (objHit == -1)
                return backgroundRadiance;
            else {
                std::vector<LightSource> lightsHit;
                Vector normal = objectList[objHit]->getNormal(vPoint[objHit]);
                Color amb = ambientComponent( objectList[objHit], backgroundRadiance );

                // shadow ray origin should be slightly  different to account for rouding errors
                Point originShadowRay(vPoint[objHit].x + normal.x * 0.1f, 
                                      vPoint[objHit].y + normal.y * 0.1f,  
                                      vPoint[objHit].z + normal.z * 0.1f );

                for(it2 = lightList.begin() ; it2 < lightList.end() ; ++it2) {
                    Vector dir( originShadowRay, (*it2).getPos(), true );
                    Ray fromPointToLight(originShadowRay, dir);

                    for(it = objectList.begin() ; it < objectList.end() ; ++it) 
                        if ( originShadowRay != (*it)->intersect(fromPointToLight) ) 
                            break;

                    if ( it == objectList.end() ) // if it went through the whole loop, then it hits the light!
                        lightsHit.push_back( *it2 );
                }
/*
                // first create a ray with origin at vPoint[objHit] and direction -> normalized from point to light source
                Vector dir(originShadowRay, lightList[0].getPos());
                Ray fromPointToLight(originShadowRay, dir);

                // we will go through the objects in the world and look for intersections
                // if intersection != from origin, it hit something
                for(it = objectList.begin() ; it < objectList.end() ; ++it) {
                    if ( originShadowRay != (*it)->intersect(fromPointToLight) ) 
                        return amb;
                }
*/
                Vector view(vPoint[objHit], originRay, true);
                Color diff_spec = illuminate( objectList[objHit], view, vPoint[objHit], objectList[objHit]->getNormal(vPoint[objHit]), lightsHit);

                return amb + diff_spec;
            }
        } 
        else 
        {
            if (objHit == -1) 
                return Color(0,0,0); 
            else 
                return objectList[objHit]->getColor();
        }
    }
    

};

#endif