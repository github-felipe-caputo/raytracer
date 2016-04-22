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

    // Index of refraction
    double nr;

    // pointer to a illuminate function (could be phong, phongblinn, etc)
    Color (*illuminate)(Object*, Vector, Point, Vector, std::vector<LightSource*>) = NULL;

public:

    // All world needs to be created is an index of refraction
    // which is set as 1 by default if no value is specified
    World (double nr = 1) : nr(nr) {
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

    // For ray marching
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

        //std::cout << ray.getDirection().x << " " << ray.getDirection().y << " " << ray.getDirection().z << std::endl;

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
                std::vector<LightSource*> lightsHitList = lightsReached(originShadowRay, lightList);

                Vector view(pointHit, originRay, true);

                Color amb = ambientComponent( objectHit, backgroundRadiance, pointHit );
                Color diff_spec = illuminate( objectHit, view, pointHit, 
                        objectHit->getNormal(pointHit), lightsHitList);

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
                    if ( kt > 0 ) {
                        // Direction of incoming ray
                        Vector rayDir = ray.getDirection();
                        Vector objNormal = objectHit->getNormal(pointHit);

                        Vector normal;
                        double nit;

                        Point transmittedRayOrigin;
                        
                        //std::cout << dot(rayDir,objNormal) << std::endl;

                        // inside
                        if (dot(-1 * rayDir,objNormal) < 0) {
                            normal = -1.0 * objNormal;
                            nit = objectHit->getNr() / nr;

                            transmittedRayOrigin = Point(pointHit.x + objNormal.x * 0.1f, 
                                                         pointHit.y + objNormal.y * 0.1f,  
                                                         pointHit.z + objNormal.z * 0.1f );

                        } else { // outside
                            normal = objNormal;
                            nit = nr / objectHit->getNr();

                            // the ray needs to go out a bit inside the object to be sure
                            transmittedRayOrigin = Point(pointHit.x + objNormal.x * -0.1f, 
                                                         pointHit.y + objNormal.y * -0.1f,  
                                                         pointHit.z + objNormal.z * -0.1f );
                        }

                        double aux = 1.0 + (pow(nit,2) * (pow( dot(-1.0 * rayDir,normal) , 2) - 1.0));

                        // If Total Internal Reflection
                        if (aux < 0) {
                            // Same thing as reflected ray
                            Vector reflectedDir = reflect(rayDir, objectHit->getNormal(pointHit), VECTOR_INCOMING );
                            finalColor += kt * spawn( Ray(transmittedRayOrigin, reflectedDir), --depth);
                        } else {
                            Vector transmittedDir = nit * rayDir + (nit * dot(-1.0 * rayDir,normal) - sqrt(aux) ) * normal;
                            finalColor += kt * spawn( Ray(transmittedRayOrigin, transmittedDir), --depth);
                        }
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

        // Now for the color of the object, if it was hit or not 
        Color objectColor;

        if (illuminate != NULL) {
            if (objHit == -1) {
                objectColor = Color(0,0,0);
            } else {
                Object* objectHit = objectList[objHit];
                Point pointHit = vPoint[objHit];

                // shadow ray origin should be slightly  different to account for rounding errors
                Vector normal = objectHit->getNormal(pointHit);
                Point originShadowRay(pointHit.x + normal.x * 0.1f, 
                                      pointHit.y + normal.y * 0.1f,  
                                      pointHit.z + normal.z * 0.1f );

                // lights that are reached/hit
                std::vector<LightSource*> lightsHitList = lightsReached(originShadowRay, lightList);

                Vector view(pointHit, originRay, true);

                Color amb = ambientComponent( objectHit, backgroundRadiance, pointHit );
                Color diff_spec = illuminate( objectHit, view, pointHit, 
                        objectHit->getNormal(pointHit), lightsHitList);

                objectColor = amb + diff_spec;
            }
        }

        // Now color through sampling light
        Color inscattering;
        
        // For every light source, get intersections
        for(std::vector<LightSource*>::iterator it = lightList.begin() ; it < lightList.end() ; ++it) {
            std::vector<Point> lightIntersections;
            if (objHit == -1) {
                lightIntersections = sampleLight(ray, (*it), SAMPLE_NUM);
            } else {
                lightIntersections = sampleLight(ray, vPoint[objHit], (*it), SAMPLE_NUM);
            }

            // now for those sample values
            // lets not use the object yet
            
            for(std::vector<Point>::iterator it2 = lightIntersections.begin() ; it2 < lightIntersections.end() ; ++it2 ) {
                if (reachesLight((*it2), objectList, (*it))) {
                    double distLightPoint = distance((*it)->getPos() , (*it2));
                    double distOriginPoint = distance(originRay , (*it2));

                    double cosAngle = dot( Vector((*it)->getPos(), (*it2), true) , Vector((*it2), originRay, true) );
                    double g = 0.7;

                    //double phase = ((3.0/(16.0*PI)) * (1.0 - cosAngle*cosAngle));
                    double phase = ( (1.0/(4.0*PI)) * ( ((1.0 - g) * (1.0 - g)) / std::pow(1.0 + g*g - 2.0*g*cosAngle,3.0/2.0)) );

                    inscattering += ( phase / std::pow(distLightPoint,2)) * ((*it)->getColor()) * std::exp( -1.0 * (ka+ks) * (distLightPoint + distOriginPoint) );
                    //inscattering += (( (3.0/(16.0*PI)) * (1.0 - cosAngle*cosAngle) + (1.0/(4.0*PI)) * ( ((1.0 - g) * (1.0 - g)) / std::pow(1.0 + g*g - 2*g*cosAngle,3.0/2.0))) / (ka+ks)) * (*it)->getColor() * (1.0 - std::exp( -1.0 * (ka+ks) * distOriginPoint ));
                } else {
                    inscattering += Color(0,0,0);
                }
            }

            // Not necessary always?
            /*
            if(!lightIntersections.empty()) {
                inscattering = inscattering / lightIntersections.size();
            }
            */
        }

        Color attenuated = objectColor * std::exp(-1 * (ka + ks) * vDist[objHit] );


        //std::cout << inscattering.r << " " << inscattering.g << " " << inscattering.b << std::endl;
        
        return attenuated + inscattering; 
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

    // point p is an intersection that happened between the ray and an object
    std::vector<Point> sampleLight(Ray ray, Point p, LightSource *light, int SAMPLE_NUM) {
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
                Point newPoint = firstPoint + val * (secondPoint - firstPoint);

                if( distance(firstPoint,newPoint) < distance(firstPoint,p) )
                    samples.push_back( newPoint );
            }
        } 
        // if doesn go to the ifs, inter = 0, sample is empty

        return samples;
    }

    // checks if from point p we can reach light without hitting any other object
    bool reachesLight(Point p, std::vector<Object*> objectList, LightSource* light){
        // ray from point to light
        Ray ray(p, Vector(p,light->getPos(),true));
        double dist = distance(p,light->getPos()); // distance between point and light source

        // we will go through the objects in the world and look for intersections
        for(std::vector<Object*>::iterator it = objectList.begin() ; it < objectList.end() ; ++it) {
            Point intersect = (*it)->intersect(ray);

            // an object was hit
            if (intersect != p) {
                if (dist > distance(p, intersect)) {
                    return false;
                }
            }
                
        }

        return true;
    }
    

};

#endif