#ifndef _KDTREE_H
#define _KDTREE_H

#include <vector>
#include "object.h"
#include "mathHelper.h"

class Kdtree {

    struct node {
        // am I a leaf?
        // if leaf == true, then objectList has stuff
        // if leaf == false, objectList == null and it has front and rear
        bool leaf;

        // what subdivision is this? x, y or z?
        int subdiv;

        // subdiv value, so if subdiv = SUBDIV_X and subdivVal = 4
        // then subdiv happens at x = 4
        double subdivVal;

        // List of objects in this node
        std::vector<Object*> objectList;

        // Voxel
        Voxel v;

        node *front;
        node *rear;

        // default
        node(){}

        // this constructor to make a interior node
        node (int subdiv, double subdivVal, Voxel v, node *newFront, node *newRear) : subdiv(subdiv), subdivVal(subdivVal), v(v) {
            leaf = false;

            this->front = newFront;
            this->rear = newRear;
        }

        // use this construction to make a leaf node
        // all you need is the object list
        node (std::vector<Object*> objectList, Voxel v) : objectList(objectList), v(v) {
            leaf = true;
        }
    };

    node *root;

public:

    // default constructor
    Kdtree(){
        root = NULL;
    }

    // constructor
    Kdtree (std::vector<Object*> objectList , Voxel V) {
        root = buildKdTree(objectList, V, SUBDIV_X);
    }

    bool exists () {
        return !(root == NULL);
    }

    node* buildKdTree (std::vector<Object*> objectList, Voxel V, int currentSubdiv) {
        if (terminate(objectList)) {
            return new node(objectList, V);
        }

        // partition plane -> spatial median
        Voxel vFront = V.splitFront(currentSubdiv);
        Voxel vRear = V.splitRear(currentSubdiv);

        // Objects on new voxels
        std::vector<Object*> objectListFront;
        std::vector<Object*> objectListRear;

        for(std::vector<Object*>::iterator it = objectList.begin() ; it < objectList.end() ; ++it) {
            if ((*it)->isInside(vFront)) {
                objectListFront.push_back((*it));
            }
            if ((*it)->isInside(vRear)) {
                objectListRear.push_back((*it));
            }
        }

        // new subdiv
        int newSubDiv = (currentSubdiv + 1) % 3;

        return new node (currentSubdiv, V.splitVal(currentSubdiv), V,
            buildKdTree(objectListFront, vFront, newSubDiv), buildKdTree(objectListRear, vRear, newSubDiv) );
    }

    bool terminate (std::vector<Object*> objectList) {
        return (objectList.size() < 30);
    }

    Object* traverse (Ray ray) {
        return traverse (ray, root);
    }

    Object* traverseForLight (Ray ray, LightSource* lightSource) {
        return traverseForLight (ray, root, lightSource);
    }

    // Will return the closest object the ray hits, or NULL if it doesn't hit anything
    Object* traverse (Ray ray, node *n) {
        // if it's a leaf, try intersectoins
        if (n->leaf) {
            Point originRay = ray.getOrigin();
            Point intersection;

            std::vector<Point> vPoint;
            std::vector<double> vDist;

            // we will go through the objects in the voxel and look for intersections
            for(std::vector<Object*>::iterator it = n->objectList.begin() ; it < n->objectList.end() ; ++it) {
                intersection = (*it)->intersect(ray);
                vPoint.push_back( intersection );
                vDist.push_back( distance(originRay, intersection) );
            }

            // we find the minimum distance on vDist, which would be closest intersection
            int objHit( indexMinElement(vDist) );

            if (objHit == -1) {
                return NULL;
            }

            // obj hit
            return n->objectList[objHit];
        }

        if ( (n->v).intersect(ray, 0, 1000) ) {
            Object *a = traverse(ray, n->rear);
            Object *b = traverse(ray, n->front);
            Point origin = ray.getOrigin();

            if (a == NULL || b == NULL) {
                return (a == NULL) ? b : a;
            } else {
                double distRayA = distance(origin, a->intersect(ray));
                double distRayB = distance(origin, b->intersect(ray));

                // they are both zero, no intersection with the objects
                if (distRayA == 0 && distRayB == 0) {
                    return NULL;
                } else if (distRayA < distRayB) {
                    return a;
                } else {
                    return b;
                }
            }

        }
        return NULL;
    }

    Object* traverseForLight (Ray ray, node *n, LightSource* lightSource) {
        // if it's a leaf, try intersectoins
        if (n->leaf) {
            Point originRay = ray.getOrigin();
            Point intersection;

            std::vector<Point> vPoint;
            std::vector<double> vDist;

            double distOriginAndLight = lightSource->getMinDistance(originRay);
            
            // we will go through the objects in the voxel and look for intersections
            std::vector<Object*>::iterator it;
            for(it = n->objectList.begin() ; it < n->objectList.end() ; ++it) {
                double distOriginIntersection = distance(originRay, (*it)->intersect(ray));
                if ( !(*it)->isEmissive() && distOriginIntersection != 0 && distOriginIntersection < distOriginAndLight) {
                    break;
                }
            }

            if ( it == n->objectList.end() ) { // if it went through the whole loop, then it hits the light!
                return NULL;
            }

            // obj hit
            return (*it);
        }

        if ( (n->v).intersect(ray, 0, 1000) ) {
            Object *a = traverseForLight(ray, n->rear, lightSource);
            Object *b = traverseForLight(ray, n->front, lightSource);
            Point origin = ray.getOrigin();

            if (a == NULL || b == NULL) {
                return (a == NULL) ? b : a;
            } else {
                double distRayA = distance(origin, a->intersect(ray));
                double distRayB = distance(origin, b->intersect(ray));

                // they are both zero, no intersection with the objects
                if (distRayA == 0 && distRayB == 0) {
                    return NULL;
                } else if (distRayA < distRayB) {
                    return a;
                } else {
                    return b;
                }
            }

        }
        return NULL;
    }

};

#endif
