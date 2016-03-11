#ifndef _MATHHELPER_H
#define _MATHHELPER_H

#define PI 3.14159265

#include <cmath>

/*
 * The Color class.
 */

struct Color {
    // RGB
    float r, g, b;

    //default
    Color( float s = 0 ) : r(s), g(s), b(s) {}

    // constructors
    Color ( float r, float g, float b ) : r(r), g(g), b(b) {}

    // Non-modifying arithematic operators
    Color operator+(const Color& rhs){
        return Color(r + rhs.r, g + rhs.g, b + rhs.b);
    } 

    Color operator/(float rhs){
        return Color(r/rhs, g/rhs, b/rhs);
    } 

    Color operator*(const Color& rhs){
        return Color(r * rhs.r, g * rhs.g, b * rhs.b);
    }

    Color operator*(float rhs){
        return Color(r * rhs, g * rhs, b * rhs);
    }

    friend Color operator*(float lhs, const Color& rhs){
        return Color(lhs * rhs.r, lhs * rhs.g, lhs * rhs.b);
    }

    // Modifying arithematic operators
    Color& operator+=( const Color& rhs ) { 
        r += rhs.r;  
        g += rhs.g;  
        b += rhs.b;

        return *this; 
    }
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

    // overloading operators
    bool operator==(const Point& rhs) {
        return (x == rhs.x && y == rhs.y && z == rhs.z);
    }

    bool operator!=(const Point& rhs) {
        return !(*this == rhs);
    }
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
    Vector ( float xn, float yn, float zn, bool norm = false ) : x(xn), y(yn), z(zn) {
        if(norm) {
            float len = sqrt( x*x+y*y+z*z );
            if (len != 0.0f) {
                x = x / len;
                y = y / len;
                z = z / len;
            }
        } 
    }

    // constructor, from origin to destination
    Vector ( Point o, Point d, bool norm = false ) { 
        x = d.x - o.x;
        y = d.y - o.y;
        z = d.z - o.z; 

        if(norm) {
            float len = sqrt( x*x+y*y+z*z );
            if (len != 0.0f) {
                x = x / len;
                y = y / len;
                z = z / len;
            }
        }  
    }

    // Non-modifying arithematic operators
    Vector operator+(const Vector& rhs){
        return Vector(x + rhs.x, y + rhs.y, z + rhs.z);
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

    if (len != 0.0f) {
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

    float twoMulDotProd = 2.0f * dot(incoming, normal);

    return Vector(incoming.x - twoMulDotProd * normal.x, incoming.y - twoMulDotProd * normal.y, incoming.z - twoMulDotProd * normal.z);
}

int indexMinElement ( std::vector<float> v ) {
    float minDist = *std::max_element(v.begin(), v.end());
    int index = -1;

    for(unsigned int i = 0; i < v.size(); ++i) {
        if (v[i] != 0 && v[i] <= minDist) {
            minDist = v[i];
            index = i;
        }
    }

    return index;
}

#endif