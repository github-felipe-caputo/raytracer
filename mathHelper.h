#ifndef _MATHHELPER_H
#define _MATHHELPER_H

#include <cmath>

/*
 * The Color class.
 */

struct Color {
    // RGB
    int r, g, b;

    //default
    Color( int s = 0 ) : r(s), g(s), b(s) {}

    // constructors
    Color ( int r, int g, int b ) : r(r), g(g), b(b) {}
};

/*
 * The Point class.
 */

struct Point {
    // 3D point
    float x, y, z;

    //default
    Point ( float s = 0 ) : x(s), y(s), z(s) {}

    // constructors
    Point ( float x, float y, float z ) : x(x), y(y), z(z) {}
};

/*
 * The Vector class.
 */

struct Vector {
    // 3D vector
    float x, y, z;

    //default
    Vector ( float s = 0 ) : x(s), y(s), z(s) {}

    // constructor, considering a vector from origin -> (x,y,z)
    Vector ( float x, float y, float z ) : x(x), y(y), z(z) {}

    // constructor, from origin to destination
    Vector ( Point o, Point d ) { 
        x = d.x - o.x;
        y = d.y - o.y;
        z = d.z - o.z; 
    }
};

/*
 * The Ray class.
 */

struct Ray {
    // 3D Ray with origin and direction
    Point o;
    Vector d;

    // constructor
    Ray ( Point p, Vector v ) : o(p), d(v) {}

    Point getOrigin() {
        return o;
    }

    Vector getDirection() {
        return d;
    }
};

/*
 * Non-class functions
 */

float distance ( const Point p, const Point q ) {
    float a = p.x - q.x;
    float b = p.y - q.y;
    float c = p.z - q.z;

    return sqrt(a*a + b*b + c*c);
}

float length ( const Vector v ) {
    return sqrt( v.x*v.x+v.y*v.y+v.z*v.z );
}

void normalize ( Vector& v ) {
    float len = length(v);

    if (len != 0) {
        v.x = v.x / len;
        v.y = v.y / len;
        v.z = v.z / len;
    }
}

Vector cross ( const Vector v, const Vector u ) {
    return Vector( v.y*u.z - v.z*u.y , v.z*u.x - v.x*u.z , v.x*u.y - v.y*u.x );
}

float dot ( const Vector v , const Vector u ) {
    return ( v.x*u.x + v.y*u.y + v.z*u.z );
}

// normal should be normalized
Vector reflect ( Vector incoming, Vector normal ) {
    normalize( incoming );

    float twoMulDotProd = 2 * dot(incoming, normal);

    return Vector(incoming.x - twoMulDotProd * normal.x, incoming.y - twoMulDotProd * normal.y, incoming.z - twoMulDotProd * normal.z);
}



#endif