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
        if (terminate(objectList))
            return new node(objectList, V);

        // partition plane -> spatial median
        Voxel vFront = V.splitFront(currentSubdiv);
        Voxel vRear = V.splitRear(currentSubdiv);

        // Objects on new voxels
        std::vector<Object*> objectListFront;
        std::vector<Object*> objectListRear;

        for(std::vector<Object*>::iterator it = objectList.begin() ; it < objectList.end() ; ++it) {
            if ((*it)->isInside(vFront))
                objectListFront.push_back((*it));
            if ((*it)->isInside(vRear))
                objectListRear.push_back((*it));
        }

        // new subdiv
        int newSubDiv;
        if (currentSubdiv == SUBDIV_Z)
            newSubDiv = SUBDIV_X;
        else
            newSubDiv = currentSubdiv + 1;

        return new node (currentSubdiv, V.splitVal(currentSubdiv), V,  
            buildKdTree(objectListFront, vFront, newSubDiv), buildKdTree(objectListRear, vRear, newSubDiv) );
    }

    bool terminate (std::vector<Object*> objectList) {
        return (objectList.size() <= 1);
    }

    Object* traverse (Ray ray) {
        return traverse (ray, root);
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

            if (objHit == -1)
                return NULL;

            // obj hit
            return n->objectList[objHit];
        }
        else 
        {
            // get entry and exit point of ray box intersection 
            Point in, out;

            if ( (n->v).intersect(ray, 0, 100, in, out) ) {
                double coordEntry, coordExit;
                Object *a, *b;

                if (n->subdiv == SUBDIV_X) {
                    coordEntry = in.x;
                    coordExit = out.x;
                } else if (n->subdiv == SUBDIV_Y) {
                    coordEntry = in.y;
                    coordExit = out.y;
                } else {
                    coordEntry = in.z;
                    coordExit = out.z;
                }

                if(coordEntry <= n->subdivVal) {
                    if (coordExit < n->subdivVal) {
                        return traverse(ray, n->rear);
                    } else {
                        if (coordExit == n->subdivVal) {
                            return traverse(ray, n->rear);
                        } else {
                            a = traverse(ray, n->rear);
                            b = traverse(ray, n->front);
                            Point origin = ray.getOrigin();

                            if (a == NULL && b == NULL) {
                                return NULL;
                            } else if (a == NULL && b != NULL) {
                                if (distance(origin, b->intersect(ray)) == 0)
                                    return NULL;
                                else
                                    return b;
                            } else if (a != NULL && b == NULL) {
                                if (distance(origin, a->intersect(ray)) == 0)
                                    return NULL;
                                else
                                    return a;
                            } else {
                                double distRayA = distance(origin, a->intersect(ray));
                                double distRayB = distance(origin, b->intersect(ray));

                                // they are both zero, no intersection with the objects
                                if (distRayA == 0 && distRayB == 0)
                                    return NULL;
                                else if (distRayA == 0 && distRayB != 0)
                                    return b;
                                else if (distRayA != 0 && distRayB == 0)
                                    return a;
                                else if (distRayA < distRayB)
                                    return a;
                                else
                                    return b;
                            }
                        }
                    }
                } else {
                    if (coordExit > n->subdivVal) {
                        return traverse(ray, n->front);
                    } else {
                        a = traverse(ray, n->front);
                        b = traverse(ray, n->rear);
                        Point origin = ray.getOrigin();

                        if (a == NULL && b == NULL) {
                            return NULL;
                        } else if (a == NULL && b != NULL) {
                            if (distance(origin, b->intersect(ray)) == 0)
                                return NULL;
                            else
                                return b;
                        } else if (a != NULL && b == NULL) {
                            if (distance(origin, a->intersect(ray)) == 0)
                                return NULL;
                            else
                                return a;
                        } else {
                            double distRayA = distance(origin, a->intersect(ray));
                            double distRayB = distance(origin, b->intersect(ray));

                            // they are both zero, no intersection with the objects
                            if (distRayA == 0 && distRayB == 0)
                                return NULL;
                            else if (distRayA == 0 && distRayB != 0)
                                return b;
                            else if (distRayA != 0 && distRayB == 0)
                                return a;
                            else if (distRayA < distRayB)
                                return a;
                            else
                                return b;
                        }
                    }
                }
            } else {
                return NULL;
            }

        }
    }
/*

    Point traverse (Ray ray, node *n) {

        // if it's a leaf, try intersectoins
        if (n->leaf) {
            std::cout << "here???" << std::endl;
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

            // object hit is
            // n->objectList[objHit];

            if (objHit == -1)
                return ray.getOrigin();

            // point hit
            return vPoint[objHit];
        }
        else 
        {
            // get entry and exit point of ray box intersection 
            Point in;
            Point out;

            if ( (n->v).intersect(ray, 0, 100, in, out) ) {

                double coordEntry, coordExit;
                Point a, b;

                if (n->subdiv == SUBDIV_X) {
                    coordEntry = in.x;
                    coordExit = out.x;
                } else if (n->subdiv == SUBDIV_Y) {
                    coordEntry = in.y;
                    coordExit = out.y;
                } else {
                    coordEntry = in.z;
                    coordExit = out.z;
                }

                if(coordEntry <= n->subdivVal) {
                    if (coordExit < n->subdivVal) {
                        return traverse(ray, n->rear);
                    } else {
                        if (coordExit == n->subdivVal) {
                            // let's move they ray a little because the next intersect will return false
                            // if it's tangent to a plane
                            Point o = ray.getOrigin();
                            Vector d = ray.getDirection();
                            Ray newRay;

                            if (n->subdiv == SUBDIV_X) {
                                newRay = Ray(Point(o.x-0.0001,o.y,o.z) , d);
                            } else if (n->subdiv == SUBDIV_Y) {
                                newRay = Ray(Point(o.x,o.y-0.0001,o.z) , d);
                            } else {
                                newRay = Ray(Point(o.x,o.y,o.z-0.0001) , d);
                            }

                            return traverse(newRay, n->rear);
                        } else {
                            Ray newRay(rayPlaneIntersection(ray,n->subdiv,n->subdivVal), ray.getDirection() );
                            a = traverse(ray, n->rear);
                            b = traverse(newRay, n->front);

                            if (a == ray.getOrigin() && b != newRay.getOrigin())
                                return b;
                            else if (a != ray.getOrigin() && b == newRay.getOrigin())
                                return a;

                            if (distance(ray.getOrigin() , a) < distance(ray.getOrigin() , b))
                                return a;
                            else
                                return b;

                        }
                    }
                } else {
                    if (coordExit > n->subdivVal) {
                        return traverse(ray, n->front);
                    } else {
                        Ray newRay(rayPlaneIntersection(ray,n->subdiv,n->subdivVal), ray.getDirection() );
                        a = traverse(ray, n->front);
                        b = traverse(newRay, n->rear);

                        if (a == ray.getOrigin() && b != newRay.getOrigin())
                            return b;
                        else if (a != ray.getOrigin() && b == newRay.getOrigin())
                            return a;

                        if (distance(ray.getOrigin() , a) < distance(ray.getOrigin() , b))
                            return a;
                        else
                            return b;
                    }
                }
            } else {
                return ray.getOrigin();
            }

        }
    }
*/
    Point rayPlaneIntersection (Ray ray, int subdiv, double val) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        Vector n;

        if (subdiv == SUBDIV_X){
            n = Vector(1,0,0);
        } else if (subdiv == SUBDIV_Y){
            n = Vector(0,1,0);
        } else {
            n = Vector(0,0,1);
        }

        // ray-plane intersection
        double w = -(n.x*o.x + n.y*o.y + n.z*o.z + val) / (n.x*d.x + n.y*d.y + n.z*d.z);

        double wx = o.x + d.x * w;
        double wy = o.y + d.y * w;
        double wz = o.z + d.z * w;  

        return Point(wx, wy, wz);
    }

};

#endif