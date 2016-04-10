#ifndef _WORLD_H
#define _WORLD_H

#include <iostream>

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

    // For ray marching we add a participant media, so the world gets a
    double ks; // media scattering coefficient
    double ka; // media absorption coefficient

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

    void addParticipantMedia(double nks, double nka) {
        ks = nks;
        ka = nka;
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
                    // double kt = objectHit->getKt();

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
    }

    Color spawnRayMarch ( Ray ray, int SAMPLE_NUM ) {
        Point originRay = ray.getOrigin();
        Point intersection;

        std::vector<Point> vPoint;
        std::vector<double> vDist;
        unsigned int i = 0;

        // first just a test with the light
        // assuming:
        //  Only a spot light, or it will break
        //  No objects for now, not even the floor, ignore objHit

        // we will go through the objects in the world and look for intersections
        for(std::vector<Object*>::iterator it = objectList.begin() ; it < objectList.end() ; ++it, ++i) {
            intersection = (*it)->intersect(ray);
            vPoint.push_back( intersection );
            vDist.push_back( distance(originRay, intersection) );
        }
        
        // we find the minimum distance on vDist, which would be closest intersection
        int objHit( indexMinElement(vDist) );

        Color inscattering;
        
        // For every light source, get intersections
        for(std::vector<LightSource*>::iterator it = lightList.begin() ; it < lightList.end() ; ++it) {
            std::vector<Point> lightIntersections = sampleLight(ray, (*it), SAMPLE_NUM);

            // now for those sample values
            // lets not use the object yet
            
            for(std::vector<Point>::iterator it2 = lightIntersections.begin() ; it2 < lightIntersections.end() ; ++it2 ) {
                double distLightPoint = distance((*it)->getPos() , (*it2));
                double distOriginPoint = distance(originRay , (*it2));

                //inscattering += ks * (1.0/(4.0*PI)) * ((*it)->getColor() / std::pow(distLightPoint,2)) * std::exp( -1 * (ka+ks) * (distLightPoint + distOriginPoint) );
            
                double cosAngle = dot( Vector((*it)->getPos(), (*it2), true) , Vector((*it2), originRay, true) );
                double g = 0.7;

                inscattering += ((3.0/(16.0*PI) * (1 - cosAngle*cosAngle) +(1.0/(4.0*PI)) * ((1 - g) / std::pow(1 + g*g - 2*g*cosAngle,3.0/2.0))) / (ka+ks)) * (*it)->getColor() * (1 - std::exp( -1 * (ka+ks) * distOriginPoint ));
            }

            if(!lightIntersections.empty()) {
                inscattering = inscattering / lightIntersections.size();
            }
        }

        //std::cout << inscattering.r << " " << inscattering.g << " " << inscattering.b << std::endl;
        
        return inscattering; 
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

    // Given a ray and a light, this function will check the intersection of that
    // ray and that light. If we get more than 1 intersection, we will uniformly sample
    // the points between the intersections.
    std::vector<Point> sampleLight(Ray ray, LightSource *light, int SAMPLE_NUM) {
        std::vector<Point> samples;
        std::vector<Point> inters = light->intersect(ray);

        if(inters.size() == 1) { // tangent to the spot light
            samples.push_back( inters[0] );
        }
        else if (inters.size() == 2) { // goes through the spot light
            Point firstPoint = inters[0];
            Point secondPoint = inters[1];

            for (int i = 1; i < SAMPLE_NUM; ++i) {
                double val = i * (1.0/SAMPLE_NUM);
                samples.push_back( firstPoint + val * (secondPoint - firstPoint) );
            }
        } 
        // if doesn go to the ifs, inter = 0, sample is empty

        return samples;
    }
    

};

#endif