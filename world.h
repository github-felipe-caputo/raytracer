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
    std::vector<LightSource*> lightList;

    // Attribute for phong illumination
    Color backgroundRadiance;

    // pointer to a illuminate function (could be phong, phongblinn, etc)
    Color (*illuminate)(Object*, Vector, Point, Vector, std::vector<LightSource*>) = NULL;

public:

    World () {
    }

    void setUpPhongIllumination(Color amLight) {
        backgroundRadiance = amLight;
        illuminate = illuminatePhong; // from "illuminationModel.h"
    }

    void setUpPhongBlinnIllumination(Color amLight) {
        backgroundRadiance = amLight;
        illuminate = illuminatePhongBlinn; // from "illuminationModel.h"
    }

    void addObject(Object *obj) {
        objectList.push_back(obj);
    }

    void addLight(LightSource *light) {
        lightList.push_back(light);
    }

    // Spawn will return the color we should use for the pixel in the ray
    Color spawn( Ray ray, int depth ) {
        Point originRay = ray.getOrigin();
        Point intersection;

        std::vector<Point> vPoint;
        std::vector<double> vDist;
        unsigned int i = 0;

        // we will go through the objects in the world and look for intersections
        for(std::vector<Object*>::iterator it = objectList.begin() ; it < objectList.end() ; ++it, ++i) {
            intersection = (*it)->intersect(ray);
            vPoint.push_back( intersection );
            vDist.push_back( distance(originRay, intersection) );
        }
        
        // we find the minimum distance on vDist, which would be closest intersection
        int objHit( indexMinElement(vDist) );

        if (illuminate != NULL) {
            if (objHit == -1) {
                return backgroundRadiance;
            } else {
                Object* objectHit = objectList[objHit];
                Point pointHit = vPoint[objHit];

                // shadow ray origin should be slightly  different to account for rounding errors
                Vector normal = objectHit->getNormal(pointHit);
                Point originShadowRay(pointHit.x + normal.x * 0.1f, 
                                      pointHit.y + normal.y * 0.1f,  
                                      pointHit.z + normal.z * 0.1f );

                // lights that are reached/hit
                std::vector<LightSource*> lightsHit = lightsReached(originShadowRay, lightList);

                Vector view(pointHit, originRay, true);

                Color amb = ambientComponent( objectHit, backgroundRadiance, pointHit );
                Color diff_spec = illuminate( objectHit, view, pointHit, 
                        objectHit->getNormal(pointHit), lightsHit);

                Color finalColor = amb + diff_spec;
                
                if ( depth > 1 ) {
                    double kr = objectHit->getKr();
                    double kt = objectHit->getKt();

                    if ( kr > 0 ) {
                        // Direction of incoming ray
                        Vector rayDir = ray.getDirection();

                        // Reflection of the ray direction
                        Vector reflectedDir = reflect(rayDir, objectHit->getNormal(pointHit), VECTOR_INCOMING );

                        // Recursion !
                        finalColor += kr * spawn( Ray(originShadowRay, reflectedDir) , --depth);
                    }
                    /*
                    if ( kt > 0 ) {
                        // transmission ray
                        finalColor += kt * spawn(reflectionRay, --depth);
                    }
                    */
                    
                }

                return finalColor;
            }
        } 
        else 
        {
            if (objHit == -1) 
                return Color(0,0,0); 
            else 
                return objectList[objHit]->getColor(vPoint[objHit]);
        }

        /*

        if( phongIllumination || phongIlluminationBlinn ) 
        {
            if (objHit == -1) {
                return backgroundRadiance;
            } else {
                // Local ilumination
                Color amb = ambientComponent( objectList[objHit], backgroundRadiance, vPoint[objHit] );

                // shadow ray origin should be slightly  different to account for rounding errors
                Vector normal = objectList[objHit]->getNormal(vPoint[objHit]);
                Point originShadowRay(vPoint[objHit].x + normal.x * 0.1f, 
                                      vPoint[objHit].y + normal.y * 0.1f,  
                                      vPoint[objHit].z + normal.z * 0.1f );

                // lights that are reached/hit
                std::vector<LightSource*> lightsHit = lightsReached(originShadowRay, lightList);

                Vector view(vPoint[objHit], originRay, true);
                Color diff_spec;

                if( phongIllumination ) {
                    diff_spec = illuminatePhong( objectList[objHit], view, vPoint[objHit], 
                        objectList[objHit]->getNormal(vPoint[objHit]), lightsHit);
                } else {
                    diff_spec = illuminatePhongBlinn( objectList[objHit], view, vPoint[objHit], 
                        objectList[objHit]->getNormal(vPoint[objHit]), lightsHit);
                }

                ////// new stuff
                Color finalColor = amb + diff_spec;

                if ( depth < 1 ) {
                    double kr = objectList[objHit]->getKr();
                    double kt = objectList[objHit]->getKt();

                    if (kr > 0) {
                        // reflection ray
                        finalColor += kr * illuminate(reflectionRay, --depth);
                    }

                    if (kt > 0) {
                        // reflection ray
                        finalColor += kt * illuminate(reflectionRay, --depth);
                    }
                }


                return finalColor;
            }
        } 
        else 
        {
            if (objHit == -1) 
                return Color(0,0,0); 
            else 
                return objectList[objHit]->getColor(vPoint[objHit]);
        }

        */
    }

    // This returns a vector of which lights the shadow ray coming from originShadowRay can reach
    std::vector<LightSource*> lightsReached(Point originShadowRay, std::vector<LightSource*> lightList){
        std::vector<LightSource*> lightsHit;
        std::vector<Object*>::iterator it;

        // For every light source, let's see if a ray from originShadowRay can reach it
        for(std::vector<LightSource*>::iterator it2 = lightList.begin() ; it2 < lightList.end() ; ++it2) {

            // If this ray can actually reach the lights 
            // (can always reach a point light, maybe not a spot light)
            if( (*it2)->reaches(originShadowRay) ) {
                Vector dir( originShadowRay, (*it2)->getPos(), true );
                Ray fromPointToLight(originShadowRay, dir);

                for(it = objectList.begin() ; it < objectList.end() ; ++it) 
                    if ( originShadowRay != (*it)->intersect(fromPointToLight) ) 
                        break;

                if ( it == objectList.end() ) // if it went through the whole loop, then it hits the light!
                    lightsHit.push_back( *it2 );
            }
        }

        return lightsHit;
    }
    

};

#endif