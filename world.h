#ifndef _WORLD_H
#define _WORLD_H

#include <iostream>

#include <vector>
#include <map>
#include <algorithm>
#include "mathHelper.h"
#include "object.h"
#include "lightSource.h"
#include "illuminationModel.h"
#include "kdtree.h"

// ray marching
#define CONSTANT_DENSITY 0
#define VARIABLE_DENSITY 1

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
    int density;

    // Index of refraction
    double nr;

    // pointer to a illuminate function (could be phong, phongblinn, etc)
    Color (*illuminate)(Object*, Vector, Point, Vector, std::map<LightSource*, std::vector<Point> >);// = NULL;

    // tree
    Kdtree kd;

public:

    // All world needs to be created is an index of refraction
    // which is set as 1 by default if no value is specified
    World (double nr = 1) : nr(nr) {
        illuminate = NULL;
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
    void addParticipantMedia(double nks, double nka, int ndensity) {
        ks = nks;
        ka = nka;
        density = ndensity;
    }

    // Creates a KDtree based on the added objects,
    // uses as the main voxel the values passed for now
    void createKdTree(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax ) {
        kd = Kdtree(objectList, Voxel(xmin,xmax,ymin,ymax,zmin,zmax));
    }

    Color spawn ( Ray ray, int depth ) {
        if (illuminate == NULL) {
            std::cerr << "Error: World needs to have illumination setup before rendering." << std::endl;
            exit(1);
        }

        if ( kd.exists() ) {
            return spawnKdtree(ray, depth);
        }
        else {
            return spawnIlluminated(ray, depth);
        }

    }

    // Spawn will return the color we should use for the pixel in the ray
    Color spawnKdtree( Ray ray, int depth ) {
        Point originRay = ray.getOrigin();

        // walk through the tree, get the object the ray hits
        Object* objectHit = kd.traverse(ray);

        // if nothing was hit
        if (objectHit == NULL) {
            return backgroundRadiance;
        } else {
            Point pointHit = objectHit->intersect(ray);

            // if object is emissive, return emissive color and end
            if (objectHit->isEmissive()) {
                return objectHit->getEmissiveColor();
            }

            // shadow ray origin should be slightly  different to account for rounding errors
            Vector normal = objectHit->getNormal(pointHit);
            Point originShadowRay(pointHit.x + normal.x * 0.001,
                                  pointHit.y + normal.y * 0.001,
                                  pointHit.z + normal.z * 0.001 );

            // the new function we will use
            std::map<LightSource*, std::vector<Point> > lightsAndPointsReachedMap = lightsReached(originShadowRay, lightList);

            Vector view(pointHit, originRay, true);

            Color amb = ambientComponent( objectHit, backgroundRadiance, pointHit );
            Color diff_spec = illuminate( objectHit, view, pointHit,
                    objectHit->getNormal(pointHit), lightsAndPointsReachedMap);

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
                    finalColor += kr * spawnKdtree( Ray(originShadowRay, reflectedDir) , depth-1);
                }
                if ( kt > 0 ) {
                    // Direction of incoming ray
                    Vector rayDir = ray.getDirection();
                    Vector objNormal = objectHit->getNormal(pointHit);

                    Vector normal;
                    double nit;

                    Point transmittedRayOrigin;

                    // inside
                    if (dot(-1 * rayDir,objNormal) < 0) {
                        normal = -1.0 * objNormal;
                        nit = objectHit->getNr() / nr;

                        transmittedRayOrigin = Point(pointHit.x + objNormal.x * 0.01,
                                                     pointHit.y + objNormal.y * 0.01,
                                                     pointHit.z + objNormal.z * 0.01 );

                    } else { // outside
                        normal = objNormal;
                        nit = nr / objectHit->getNr();

                        // the ray needs to go out a bit inside the object to be sure
                        transmittedRayOrigin = Point(pointHit.x + objNormal.x * -0.01,
                                                     pointHit.y + objNormal.y * -0.01,
                                                     pointHit.z + objNormal.z * -0.01 );
                    }

                    double aux = 1.0 + (pow(nit,2) * (pow( dot(-1.0 * rayDir,normal) , 2) - 1.0));

                    // If Total Internal Reflection
                    if (aux < 0) {
                        // Same thing as reflected ray
                        Vector reflectedDir = reflect(rayDir, normal, VECTOR_INCOMING );
                        finalColor += kt * spawnKdtree( Ray(transmittedRayOrigin, reflectedDir), depth-1);
                    } else {
                        Vector transmittedDir = nit * rayDir + (nit * dot(-1.0 * rayDir,normal) - sqrt(aux) ) * normal;
                        finalColor += kt * spawnKdtree( Ray(transmittedRayOrigin, transmittedDir), depth-1);
                    }
                }
            }
            return finalColor;
        }
    }

    // Spawn will return the color we should use for the pixel in the ray
    Color spawnIlluminated( Ray ray, int depth ) {
        Point originRay = ray.getOrigin();
        Point intersection;

        std::vector<Point> vPoint;
        std::vector<double> vDist;

        // we will go through the objects in the world and look for intersections
        for(std::vector<Object*>::iterator it = objectList.begin() ; it < objectList.end() ; ++it) {
            intersection = (*it)->intersect(ray);
            vPoint.push_back( intersection );
            vDist.push_back( distance(originRay, intersection) );
        }

        // we find the minimum distance on vDist, which would be closest intersection
        int objHit( indexMinElement(vDist) );

        // if no object was hit
        if (objHit == -1) {
            return backgroundRadiance;
        } else {
            Object* objectHit = objectList[objHit];
            Point pointHit = vPoint[objHit];

            // if object is emissive, return emissive color and end
            if (objectHit->isEmissive()) {
                return objectHit->getEmissiveColor();
            }

            // shadow ray origin should be slightly  different to account for rounding errors
            Vector normal = objectHit->getNormal(pointHit);
            Point originShadowRay(pointHit.x + normal.x * 0.01,
                                  pointHit.y + normal.y * 0.01,
                                  pointHit.z + normal.z * 0.01 );

            // the new function we will use
            std::map<LightSource*, std::vector<Point> > lightsAndPointsReachedMap = lightsReached(originShadowRay, lightList);

            Vector view(pointHit, originRay, true);

            Color amb = ambientComponent( objectHit, backgroundRadiance, pointHit );
            Color diff_spec = illuminate( objectHit, view, pointHit,
                    objectHit->getNormal(pointHit), lightsAndPointsReachedMap);

            Color finalColor = amb + diff_spec;

            // If lights hit is empty, it means the shadow ray might have hit something
            // before reaching the light, i.e. an object
            // In this case we should take into account if the object is transparent
            /* TODO: This cheat for light through transparent objects still not fully working
               I'm leaving it here for future Felipe to figure something out
            if ( !allRaysHitLight(lightsAndPointsReachedMap) ) {

                std::map<LightSource*, std::vector<Point> > lightsAndPointsReachedMapTransp = lightsReachedThroughTransparency(originShadowRay,
                                                                                                                lightsAndPointsReachedMap);

                Color diff_spec = illuminate( objectHit, view, pointHit,
                        objectHit->getNormal(pointHit), lightsAndPointsReachedMapTransp);

                finalColor += 0.8 * diff_spec;
            }
            */

            if ( depth > 1 ) {
                double kr = objectHit->getKr();
                double kt = objectHit->getKt();

                if ( kr > 0 ) {
                    // Direction of incoming ray
                    Vector rayDir = ray.getDirection();

                    // Reflection of the ray direction
                    Vector reflectedDir = reflect(rayDir, objectHit->getNormal(pointHit), VECTOR_INCOMING );

                    // Recursion !
                    finalColor += kr * spawnIlluminated( Ray(originShadowRay, reflectedDir) , depth-1);
                }
                if ( kt > 0 ) {
                    // Direction of incoming ray
                    Vector rayDir = ray.getDirection();
                    Vector objNormal = objectHit->getNormal(pointHit);

                    Vector normal;
                    double nit;

                    Point transmittedRayOrigin;

                    // inside
                    if (dot(-1 * rayDir,objNormal) < 0) {
                        normal = -1.0 * objNormal;
                        nit = objectHit->getNr() / nr;

                        transmittedRayOrigin = Point(pointHit.x + objNormal.x * 0.01,
                                                     pointHit.y + objNormal.y * 0.01,
                                                     pointHit.z + objNormal.z * 0.01 );

                    } else { // outside
                        normal = objNormal;
                        nit = nr / objectHit->getNr();

                        // the ray needs to go out a bit inside the object to be sure
                        transmittedRayOrigin = Point(pointHit.x + objNormal.x * -0.01,
                                                     pointHit.y + objNormal.y * -0.01,
                                                     pointHit.z + objNormal.z * -0.01 );
                    }

                    double aux = 1.0 + (pow(nit,2) * (pow( dot(-1.0 * rayDir,normal) , 2) - 1.0));

                    // If Total Internal Reflection
                    if (aux < 0) {
                        // Same thing as reflected ray
                        Vector reflectedDir = reflect(rayDir, normal, VECTOR_INCOMING );
                        finalColor += kt * spawnIlluminated( Ray(transmittedRayOrigin, reflectedDir), depth-1);
                    } else {
                        Vector transmittedDir = nit * rayDir + (nit * dot(-1.0 * rayDir,normal) - sqrt(aux) ) * normal;
                        finalColor += kt * spawnIlluminated( Ray(transmittedRayOrigin, transmittedDir), depth-1);
                    }
                }
            }

            return finalColor;
        }

    }
/*
    Color spawnRayMarch ( Ray ray, int SAMPLE_NUM ) {
        Point originRay = ray.getOrigin();
        Point intersection;

        std::vector<Point> vPoint;
        std::vector<double> vDist;

        // first just a test with the light
        // assuming:
        //  Only a spot light, or it will break
        //  No objects for now, not even the floor, ignore objHit

        // we will go through the objects in the world and look for intersections
        for(std::vector<Object*>::iterator it = objectList.begin() ; it < objectList.end() ; ++it) {
            intersection = (*it)->intersect(ray);
            vPoint.push_back( intersection );
            vDist.push_back( distance(originRay, intersection) );
        }

        // we find the minimum distance on vDist, which would be closest intersection
        int objHit( indexMinElement(vDist) );

        // For the color of the object, if it was hit or not
        Color objectColor;

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

        // for one light source
        //if ( !lightList[0]->reaches(vPoint[objHit]) )
          // objectColor = Color(0,0,0);

        // Now color through sampling light
        Color inscattering;

        // For every light source, get intersections
        for(std::vector<LightSource*>::iterator it = lightList.begin() ; it < lightList.end() ; ++it) {
            std::vector<Point> lightIntersections;
            if (objHit == -1) {
                lightIntersections = samplePointLight(ray, (*it), SAMPLE_NUM);
            } else {
                lightIntersections = samplePointLight(ray, vPoint[objHit], (*it), SAMPLE_NUM);
            }

            // now for those sample values
            // lets not use the object yet

            for(std::vector<Point>::iterator it2 = lightIntersections.begin() ; it2 < lightIntersections.end() ; ++it2 ) {
                if (reachesLight((*it2), objectList, (*it))) {
                    double distLightPoint = distance((*it)->getPos() , (*it2));
                    double distOriginPoint = distance(originRay , (*it2));

                    double cosAngle = dot( Vector((*it)->getPos(), (*it2), true) , Vector((*it2), originRay, true) );

                    if ( density == CONSTANT_DENSITY ) {
                        double g = 0.3;
                        inscattering += (( (3.0/(16.0*PI)) * (1.0 - cosAngle*cosAngle) + (1.0/(4.0*PI)) * ( ((1.0 - g) * (1.0 - g)) / std::pow(1.0 + g*g - 2*g*cosAngle,1.5))) / (ka+ks)) * (*it)->getColor() * (1.0 - std::exp( -1.0 * (ka+ks) * distOriginPoint ));
                    } else {
                        double phase = ((3.0/(16.0*PI)) * (1.0 - cosAngle*cosAngle));
                        inscattering += ( phase / std::pow(distLightPoint,2)) * ((*it)->getColor()) * std::exp( -1.0 * (ka+ks) * (distLightPoint + distOriginPoint) );
                    }
                }
            }

            if(!lightIntersections.empty()) {
                if (density == CONSTANT_DENSITY)
                    inscattering = inscattering / lightIntersections.size();
                else
                    inscattering = 50 * inscattering / lightIntersections.size();
            }

        }

        Color attenuated = objectColor;
        if (objHit != -1) {
            attenuated = objectColor * std::exp(-1 * (ka + ks) * vDist[objHit] );
        }

        return attenuated + inscattering;
    }
*/
    // This returns a map of which lights the shadow ray coming from originShadowRay can reach
    // and which points it actually hit on the light (necessary for area lights)
    std::map<LightSource*, std::vector<Point> > lightsReached(Point originShadowRay, std::vector<LightSource*> lightList){
        std::vector<LightSource*> lightsHit;
        std::vector<Object*>::iterator itObj;

        std::map<LightSource*, std::vector<Point>> result;
        std::vector<Point> pointsHitOnLight;

        // For every light source, let's see if a ray from originShadowRay can reach it
        for(std::vector<LightSource*>::iterator it = lightList.begin() ; it < lightList.end() ; ++it) {

            // If this ray can actually reach the lights
            // (can always reach a point light, maybe not a spot light)
            if( (*it)->reaches(originShadowRay) ) {
                // could be an area light
                double distOriginAndLight = (*it)->getMinDistance(originShadowRay);
                std::vector<Point> pointsOnLightSurface = (*it)->getPos();
                pointsHitOnLight.clear();
                for(std::vector<Point>::iterator it2 = pointsOnLightSurface.begin() ; it2 < pointsOnLightSurface.end() ; ++it2) {
                    Vector dir( originShadowRay, (*it2), true );
                    Ray fromPointToLight(originShadowRay, dir);

                    // TODO: better to check if the lght is an area light then simply intersect with it and get distance

                    for(itObj = objectList.begin() ; itObj < objectList.end() ; ++itObj) {
                        double distOriginIntersection = distance(originShadowRay, (*itObj)->intersect(fromPointToLight));
                        if ( !(*itObj)->isEmissive() && distOriginIntersection != 0
                            && distOriginIntersection < distOriginAndLight) { // emissive object should not block, it's light
                            break;
                        }
                    }

                    if ( itObj == objectList.end() ) { // if it went through the whole loop, then it hits the light!
                        pointsHitOnLight.push_back( *it2 );
                    }
                }

                if ( !pointsHitOnLight.empty() ) {
                    result.insert(std::pair<LightSource*, std::vector<Point> >(*it, pointsHitOnLight));
                }

            }
        }

        return result;
    }

    // This function will check the lightsAndPointsReachedMap to see if the number of
    // samples on a light are the same as the number of points hit
    // because if they are, then nothing needs to be done
    // but if they are not, maybe one of the rays that tried to hit the samples
    // went through a transparent object, so we check further
    bool allRaysHitLight(std::map<LightSource*, std::vector<Point> > lightsAndPointsReachedMap) {
        if (lightsAndPointsReachedMap.empty()){
            return false;
        }

        for (std::map<LightSource*, std::vector<Point> >::iterator it=lightsAndPointsReachedMap.begin(); it!=lightsAndPointsReachedMap.end(); ++it) {
            LightSource *lightHit = (it->first);
            std::vector<Point> pointsHit = (it->second);

            if (pointsHit.size() < lightHit->getNumSamplesOnSurface()) {
                // If points hit is less than the num samples of surface, then dis difference
                // are the shadow rays we need to shoot again and see if they go through transparent objetcs
                return false;
            }
        }

        return true;
    }

    // This returns a vector of which lights the shadow ray coming from originShadowRay can reach
    // But in this case, if there is a transparent object in the way, we consider that the
    // light is still reachable
    std::map<LightSource*, std::vector<Point> > lightsReachedThroughTransparency(Point originShadowRay,
                            std::map<LightSource*, std::vector<Point> > lightsAndPointsReachedMap) {
        std::vector<Object*>::iterator itObj;

        std::map<LightSource*, std::vector<Point>> result;
        std::vector<Point> pointsHitOnLight;

        // if the map is empty, no light was reached, i.e, this point is in complete shadow
        // in this case, we need to shoot all points for all lights!
        if (lightsAndPointsReachedMap.empty()) {

            for(std::vector<LightSource*>::iterator it = lightList.begin() ; it < lightList.end() ; ++it) {

                // If this ray can actually reach the lights
                // (can always reach a point light, maybe not a spot light)
                if( (*it)->reaches(originShadowRay) ) {
                    // could be an area light
                    std::vector<Point> pointsOnLightSurface = (*it)->getPos();
                    for(std::vector<Point>::iterator it2 = pointsOnLightSurface.begin() ; it2 < pointsOnLightSurface.end() ; ++it2) {
                        Vector dir( originShadowRay, (*it2), true );
                        Ray fromPointToLight(originShadowRay, dir);

                        for(itObj = objectList.begin() ; itObj < objectList.end() ; ++itObj) {
                            if ( !(*itObj)->isEmissive() && // ignore emissive objects, our area lights
                                 (*itObj)->getKt() == 0 &&  // only consider if object transparency = 0 (not transparent at all)
                                 originShadowRay != (*itObj)->intersect(fromPointToLight) ) {
                                break;
                            }
                        }

                        if ( itObj == objectList.end() ) { // if it went through the whole loop, then it hits the light!
                            pointsHitOnLight.push_back( *it2 );
                        }
                    }

                    if ( !pointsHitOnLight.empty() ) {
                        result.insert(std::pair<LightSource*, std::vector<Point> >(*it, pointsHitOnLight));
                        pointsHitOnLight.clear();
                    }

                }
            }

            return result;

        }



        for (std::map<LightSource*, std::vector<Point> >::iterator it=lightsAndPointsReachedMap.begin(); it!=lightsAndPointsReachedMap.end(); ++it) {
            LightSource *lightHit = (it->first);
            std::vector<Point> pointsHit = (it->second);

            if (pointsHit.size() < lightHit->getNumSamplesOnSurface()) {
                // If points hit is less than the num samples of surface, then dis difference
                // are the shadow rays we need to shoot again and see if they go through transparent objetcs
                int numRaysDidntHitLight = lightHit->getNumSamplesOnSurface() - pointsHit.size();
                std::vector<Point> pointsOnLightSurface = lightHit->getPos();

                std::vector<Point>::iterator it2 = pointsOnLightSurface.begin();
                for(int i = 0; i < numRaysDidntHitLight && it2 < pointsOnLightSurface.end() ; ++it2, ++i) {
                    Vector dir( originShadowRay, (*it2), true );
                    Ray fromPointToLight(originShadowRay, dir);

                    for(itObj = objectList.begin() ; itObj < objectList.end() ; ++itObj) {
                        if ( !(*itObj)->isEmissive() && // ignore emissive objects, our area lights
                             (*itObj)->getKt() == 0 &&  // only consider if object transparency = 0 (not transparent at all)
                             originShadowRay != (*itObj)->intersect(fromPointToLight) ) {
                            break;
                        }
                    }

                    if ( itObj == objectList.end() ) { // if it went through the whole loop, then it hits the light!
                        pointsHitOnLight.push_back( *it2 );
                    }
                }

                if ( !pointsHitOnLight.empty() ) {
                    result.insert(std::pair<LightSource*, std::vector<Point> >(lightHit, pointsHitOnLight));
                    pointsHitOnLight.clear();
                }
            }
        }
        return result;
    }
/*
    // Given a ray and a light, this function will check the intersection of that
    // ray and that light. If we get more than 1 intersection, we will uniformly sample
    // the points between the intersections.
    std::vector<Point> samplePointLight(Ray ray, LightSource *light, int SAMPLE_NUM) {
        std::vector<Point> samples;
        Point firstPoint = ray.getOrigin();
        Vector dir = ray.getDirection();
        Point secondPoint = firstPoint + 10 * Point(dir.x,dir.y,dir.z);

        for (int i = 1; i < SAMPLE_NUM; ++i) {
            double val = i * (1.0/SAMPLE_NUM);
            samples.push_back( firstPoint + val * (secondPoint - firstPoint) );
        }

        return samples;
    }

    std::vector<Point> samplePointLight(Ray ray, Point p, LightSource *light, int SAMPLE_NUM) {
        std::vector<Point> samples;
        Point firstPoint = ray.getOrigin();
        Vector dir = ray.getDirection();
        Point secondPoint = firstPoint + 10 * Point(dir.x,dir.y,dir.z);

        for (int i = 1; i < SAMPLE_NUM; ++i) {
            double val = i * (1.0/SAMPLE_NUM);
            Point newPoint = firstPoint + val * (secondPoint - firstPoint);

            if( distance(firstPoint,newPoint) > distance(firstPoint,p) )
                break;

            samples.push_back( newPoint );
        }

        return samples;
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

                if( distance(firstPoint,newPoint) > distance(firstPoint,p) )
                    break;
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
*/

};

#endif
