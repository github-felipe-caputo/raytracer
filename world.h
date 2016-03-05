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

        int objHit = -1; // need to know which object is hit so we grab its values for phong
        
        for(i = 0; i < objectList.size(); ++i) {
            auxDist = distance(ray.getOrigin(), vPoint[i]);

            if (auxDist != 0 && minDist == -1) {
                minDist = auxDist;
                finalColor = vColor[i];
                objHit = i;
            } else if (auxDist != 0 && auxDist < minDist) {
                minDist = auxDist;
                finalColor = vColor[i];
                objHit = i;
            }
        }


        // here we will have the closest point by checking vPoint[objHit]
        // se now we need to check from that point to the light, if we
        // hit anything else
/*
        // first create a ray with origin at vPoint[objHit] and direction -> normalized from point to light source
        Vector dir(vPoint[objHit], lightList[0].getPos());
        normalize(dir);
        Ray fromPointToLight(vPoint[objHit], dir);

        // we will go through the objects in the world and look for intersections
        i = 0;
        for(it = objectList.begin() ; it < objectList.end() ; ++it, ++i) {
            // if intersection is different from ray origin once, means it doesnt hit the light
            if ( vPoint[objHit] != (*it)->intersect(fromPointToLight) )
                break;
        }
*/



        if( phongIllumination ) 
        {
            if (minDist == -1)
                return backgroundRadiance;
            else {
                //return ambientComponent( objectList[objHit], backgroundRadiance );

                Vector view(vPoint[objHit], ray.getOrigin());
                normalize(view);

                //illuminate( objectList[objHit], view, vPoint[objHit], objectList[objHit].getNormal(vPoint[objHit]), lightList);
            
                Color amb = ambientComponent( objectList[objHit], backgroundRadiance );

                // ======

                // here we will have the closest point by checking vPoint[objHit]
                // se now we need to check from that point to the light, if we
                // hit anything else

                // first create a ray with origin at vPoint[objHit] and direction -> normalized from point to light source
                Vector dir(vPoint[objHit], lightList[0].getPos());
                Ray fromPointToLight(vPoint[objHit], dir);

                // we will go through the objects in the world and look for intersections
                for(it = objectList.begin() ; it < objectList.end() ; ++it) {
                    // if intersection is different from ray origin once, means it doesnt hit the light
                    // so return only ambient
                    if ( fromPointToLight.getOrigin() != (*it)->intersect(fromPointToLight) )
                        return amb;
                }

                // ======

                Color diff_spec = illuminate( objectList[objHit], view, vPoint[objHit], objectList[objHit]->getNormal(vPoint[objHit]), lightList);

                return Color(amb.r + diff_spec.r, amb.g + diff_spec.g, amb.b + diff_spec.b);
            }
                
        } 
        else 
        {
            if (minDist == -1) 
                return Color(0,0,0); 
            else 
                return finalColor;
        }


        //
        // having the point here is prob where I should check if from that point I reach the light position,
        // If I do, start the illumination stuff
        //
        
        // if there was a intersection, let's do first the ambient component
        

        // return finalColor;
    }
    

};

#endif