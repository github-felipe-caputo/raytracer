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

        // Voxel
        Voxel v;

        // what subdivision is this? x, y or z?
        int subdiv;

        // subdiv value, so if subdiv = SUBDIV_X and subdivVal = 4
        // then subdiv happens at x = 4
        double subdivVal;

        // List of objects in this node
        std::vector<Object*> objectList;

        node *front;
        node *rear;

        // default
        node(){}

        // this constructor to make a interior node
        node (int subdiv, double subdivVal, node *newFront, node *newRear) : subdiv(subdiv), subdivVal(subdivVal) {
            leaf = false;

            this->front = newFront;
            this->rear = newRear;
        }

        // use this construction to make a leaf node
        // all you need is the object list
        node (std::vector<Object*> objectList) : objectList(objectList) {
            leaf = true;
        }



    };

    node *root;

public:

    Kdtree(){}

    // default constructor
    Kdtree (std::vector<Object*> objectList , Voxel V) {
        root = buildKdTree(objectList, V, SUBDIV_X);
    }

    node* buildKdTree (std::vector<Object*> objectList, Voxel V, int currentSubdiv) {
        if (terminate(objectList, V))
            return new node(objectList);

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

        return new node (currentSubdiv, V.splitVal(currentSubdiv), 
            buildKdTree(objectListFront, vFront, newSubDiv), buildKdTree(objectListRear, vRear, newSubDiv) );
    }

    bool terminate (std::vector<Object*> objectList , Voxel V) {
        return (objectList.size() <= 1);
    }

    node* getRoot(){
        return root;
    }

    void walk (node *n) {

        if (n->leaf == true) {
            std::cout << "leaf! " << n->objectList.size() << std::endl;
            return;
        }

        walk (n->front);

        std::cout << "not leaf! " << n->subdiv << " " << n->subdivVal << std::endl;

        walk (n->rear);
    }


    void traverse (Ray ray, node *n) {

        // if it's a leaf, try intersectoins
        if (n->leaf) {

            // we will go through the objects in the voxel and look for intersections
            for(std::vector<Object*>::iterator it = n->objectList.begin() ; it < n->objectList.end() ; ++it) {
                intersection = (*it)->intersect(ray);
                vPoint.push_back( intersection );
                vDist.push_back( distance(originRay, intersection) );
            }

            // we find the minimum distance on vDist, which would be closest intersection
            int objHit( indexMinElement(vDist) );

            // object hit is
            n->objectList[objHit];

            // point hit
            vPoint[objHit];
        }
        else 
        {
            // get entry and exit point of ray box intersection 
            Point in;
            Point out;



        }




    }

};

#endif