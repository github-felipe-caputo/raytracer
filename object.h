#ifndef _OBJECT_H
#define _OBJECT_H

#include "mathHelper.h"

class Object {
protected:
    // material?
    Color col;
public:
    Object(Color col) : col(col) {}
    virtual Point interesect (Ray ray) = 0;
};

class Sphere : public Object {
    // Center and radius
    Point c;
    float r;

public:

    Sphere ( Point c, float r, Color col) :  Object(col), c(c), r(r) {}

    Point interesect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        // a = 1 because direction of a ray is normalized
        //float A = 1;
        float B = 2.0f * (d.x * (o.x - c.x) + d.y * (o.y - c.y) + d.z * (o.z - c.z));
        float C = (o.x - c.x)*(o.x - c.x) + (o.y - c.y)*(o.y - c.y) + (o.z - c.z)*(o.z - c.z) - r*r;
        float w;

        if(B*B - 4.0f*C >= 0.0f) {
            float w1 = - (B + sqrt(B*B - 4.0f*C)) / 2;
            float w2 = - (B - sqrt(B*B - 4.0f*C)) / 2;

            if (w1 > 0 && w1 <= w2) // w1 is positive and smaller than or equal to w2
                w = w1;
            else if (w2 > 0) // w2 is positive and either smaller than w1 or w1 is negative
                w = w2;      

            return Point(o.x + d.x * w, o.y + d.y * w, o.z + d.z * w);       
        }

        return o; // will return the point origin if there is no intersection
    }

    Color getColor () {
        return col;
    }
};
/*
class Quad : public Object {
    // Quadrilateral, 4 points and a normal
    // I assume they are in the same plane
    Point a, b, c, d;
    Vector n;

    // need shortest distance between origin and quad/polygon for intersection
    float f;

public:
    // four points should be in clockwise direction
    //
    //  a -- > b
    //  /\     |
    //  |      \/
    //  d < -- c
    //

    // PRO-TIP: THIS IS PROBABLE WRONG, NEED TO FIX IT (LATER)

    Quad ( Point a, Point b, Point c, Point d ) : a(a), b(b), c(c), d(d) {
        Vector v(a,b);
        Vector u(a,d);

        // get plane normal
        n = cross(v,u);

        // find value f
        float k = - (n.x * a.x + n.y * a.y + n.z * a.z);
        f = k / sqrt(n.x*n.x + n.y*n.y + n.z*n.z);

        normalize(n);
    }

    bool interesect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();

        // ray-plane intersection
        float w = -(n.x*o.x + n.y*o.y + n.z*o.z + f) / (n.x*d.x + n.y*d.y + n.z*d.z);
        float wx, wz; //wy

        // there was a intersection
        if (w > 0) {
            // actual intersection point
            wx = o.x + d.x * w;
            wz = o.z + d.z * w;

            float vertices[][3] = {{a.x,a.y,a.z},{b.x,b.y,b.z},{c.x,c.y,c.z},{d.x,d.y,d.z}};

            int i, j;
            bool result = false;
            for (i = 0, j = 3; i < 4; j = i++) {
                if ( ((vertices[i][2]>wz) != (vertices[j][2]>wz)) && 
                    (wx < (vertices[j][0]-vertices[i][0]) * (wz-vertices[i][2]) / (vertices[j][2]-vertices[i][2]) + vertices[i][0]) )
                    result = !result;
            }
            return result;
        }
        return false;
    }
};
*/

#endif