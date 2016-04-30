#ifndef _OBJECT_H
#define _OBJECT_H

#include <vector>
#include <cmath>
#include "mathHelper.h"

class Object {
protected:
    // material?

    // color, also ambient/diffuse for phong
    Color col;

    // other phong values
    Color specular;
    double ka, kd, ks, ke;

    // values for reflection and transmission
    double kr, kt;

    // value for refraction
    double nr;

public:
    // Object without solid color, called when creating textured object
    Object() {}

    Object(Color col) : col(col) {}
    
    virtual Point intersect (Ray ray) = 0;

    virtual bool isInside (Voxel v) = 0;

    virtual Vector getNormal (Point p) = 0;
    
    // this function is to get a color in a specific point, if this object has
    // a texture
    virtual Color getColor (Point p) = 0;

    Color getColor(){return col;}

    // function mainly used for the ray marching, distance
    // min distance between a point and the object
    virtual double distanceToObject (const Point p) = 0;

    // kd + ks < 1 YOU PAY ATTENTION JESUS
    void setUpPhong (Color spec, double newka, double newkd, double newks, double newke) { 
        specular = spec;
        ka = newka;
        kd = newkd;
        ks = newks;
        ke = newke; 
    }

    void setUpReflectionTransmission(double nkr, double nkt, double nnr) {
        kr = nkr;
        kt = nkt;
        nr = nnr;
    }

    Color getSpecularColor () {
        return specular;
    }

    double getKa() {
        return ka;
    }

    double getKd() {
        return kd;
    }

    double getKs() {
        return ks;
    }

    double getKe() {
        return ke;
    }

    double getKr() {
        return kr;
    }

    double getKt() {
        return kt;
    }

    double getNr(){
        return nr;
    }

};

class Sphere : public Object {
    // Center and radius
    Point c;
    double r;

    // pointer to a possible texture function
    Color (*colorFromTexture)(Point, double, Point) = NULL;

public:

    // creating an object
    // if texture = true when creating, 'col' is ignored (function getColorFromTexture will be used instead)
    Sphere ( Point c, double r, Color col) : Object(col), c(c), r(r) {
    }

    Sphere ( Point c, double r, Color (*function)(Point, double, Point) ) : c(c), r(r) {
        colorFromTexture = function;
    }

    Point intersect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        // if distance between center and ray origin is greater than
        // the radius, the point is outside the sphere
        //if( distance(c,o) > r ) {
            // a = 1 because direction of a ray is normalized
            //double A = 1;
            double B = 2.0 * (d.x * (o.x - c.x) + d.y * (o.y - c.y) + d.z * (o.z - c.z));
            double C = (o.x - c.x)*(o.x - c.x) + (o.y - c.y)*(o.y - c.y) + (o.z - c.z)*(o.z - c.z) - r*r;
            double w;

            double BBminus4C = B*B - 4.0*C;

            if (BBminus4C < 0) 
            {
                return o; // will return the point origin if there is no intersection
            }
            else if (BBminus4C == 0) 
            {
                w = (-B + 0) / 2.0;
                return Point(o.x + d.x * w, o.y + d.y * w, o.z + d.z * w);
            } 
            else 
            {
                double w1 = (-B + sqrt(BBminus4C)) / 2.0;
                double w2 = (-B - sqrt(BBminus4C)) / 2.0;

                if (w1 > 0 && w1 <= w2) // w1 is positive and smaller than or equal to w2
                    w = w1;
                else if (w2 > 0) // w2 is positive and either smaller than w1 or w1 is negative
                    w = w2;
                else if (w1 > 0)
                    w = w1;

                return Point(o.x + d.x * w, o.y + d.y * w, o.z + d.z * w);
            } 
        //} else { // if point is inside the sphere there will be ONE intersection

            //return Point(10,10,10);
        //}
    } 

    // checks if this object is inside a voxel
    // returns true if even part of the object is inside of it
    bool isInside (Voxel v) {
        double s, d = 0;

        if (c.x < v.xLeft) {
            s = c.x - v.xLeft;
            d += s*s;
        } else if (c.x > v.xRight) {
            s = c.x - v.xRight;
            d += s*s;
        }

        if (c.y < v.yBottom) {
            s = c.y - v.yBottom;
            d += s*s;
        } else if (c.y > v.yTop) {
            s = c.y - v.yTop;
            d += s*s;
        }

        if (c.z < v.zFar) {
            s = c.z - v.zFar;
            d += s*s;
        } else if (c.z > v.zNear) {
            s = c.z - v.zNear;
            d += s*s;
        }

        return (d <= r*r);
    }

    Vector getNormal (Point p) {
        Vector normal(c, p);
        normalize(normal);
        return normal;
    }

    Color getColor (Point p) {
        if (*colorFromTexture == NULL)
            return col;
        else            
            return (*colorFromTexture)(c,r,p);
    }

    double distanceToObject (const Point p) {
        return (distance(p, c) - r);
    }
};

class Polygon : public Object {
    // Polygon can have three or more vertices
    // here I assume they are definetly on the same plane
    std::vector<Point> vertices;

    // normal
    Vector n;

    // need shortest distance between origin and polygon for intersection
    double f;

    // this is a function pointer for a possible texture function,
    // it requires a vector of points (the vertices of the polygon) and a point
    // in the polygon as parameters, and returns the color of that point
    Color (*colorFromTexture)(std::vector<Point>, Point) = NULL;

public:

    // creating an object
    // if texture = true when creating, 'col' is ignored (function getColorFromTexture will be used instead)
    // normal should be normalized before constructing the polygon
    Polygon ( std::vector<Point> vert, Vector n, Color col) : Object(col), vertices(vert), n(n) {

        // for now I'm assuming all the vertices are on the same plane, and that place
        // has a normal (0,1,0), and all the points are on a plane parallel to the x and z plane
        // so f will always be y of any of the vertices (they are always the same)
        f = std::abs( vert[0].y );
    }

    // creating an object
    // instead of passing a color, pass a function for texture
    // normal should be normalized before constructing the polygon
    Polygon ( std::vector<Point> vert, Vector n, Color (*function)(std::vector<Point>, Point) ) : vertices(vert), n(n) {

        // for now I'm assuming all the vertices are on the same plane, and that place
        // has a normal (0,1,0), and all the points are on a plane parallel to the x and z plane
        // so f will always be y of any of the vertices (they are always the same)
        f = std::abs( vert[0].y );

        colorFromTexture = function;
    }    

    Point intersect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        // ray-plane intersection
        double w = -(n.x*o.x + n.y*o.y + n.z*o.z + f) / (n.x*d.x + n.y*d.y + n.z*d.z);

        // there was a intersection
        if ( w > 0.0 ) {
            // actual intersection point
            double wx = o.x + d.x * w;
            double wy = o.y + d.y * w;
            double wz = o.z + d.z * w;

            unsigned int i, j;
            bool result = false;
            for (i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
                if ( ((vertices[i].z > wz) != (vertices[j].z > wz)) && 
                    (wx < (vertices[j].x-vertices[i].x) * (wz-vertices[i].z) / (vertices[j].z-vertices[i].z) + vertices[i].x) )
                    result = !result;
            }

            if(result) 
                return Point(wx, wy, wz);
        } 

        return o;
    }

    // checks if this object is inside a voxel
    // returns true if even part of the object is inside of it
    bool isInside (Voxel v) {
        return true;
    }

    Vector getNormal (Point p) {
        return n;
    }

    Color getColor (Point p) {
        if (*colorFromTexture == NULL)
            return col;
        else
            return (*colorFromTexture)(vertices,p);
    }

    // for now it's a simple point plane distance, I should fix it later to 
    // actual point polygon distance
    double distanceToObject (const Point p) {
        return (n.x * p.x + n.y * p.y + n.z * p.z + f) / (sqrt(n.x*n.x + n.y*n.y + n.z*n.z));
    }
};


#endif