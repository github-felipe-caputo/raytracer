#ifndef _MATHHELPER_H
#define _MATHHELPER_H

#define PI 3.14159265

#include <cmath>

/*
 * The Color class.
 */

struct Color {
    // RGB
    double r, g, b;

    //default
    Color( double s = 0 ) : r(s), g(s), b(s) {}

    // constructors
    Color ( double r, double g, double b ) : r(r), g(g), b(b) {}

    // Non-modifying arithematic operators
    Color operator+(const Color& rhs){
        return Color(r + rhs.r, g + rhs.g, b + rhs.b);
    } 

    Color operator/(double rhs){
        return Color(r/rhs, g/rhs, b/rhs);
    } 

    Color operator*(const Color& rhs){
        return Color(r * rhs.r, g * rhs.g, b * rhs.b);
    }

    Color operator*(double rhs){
        return Color(r * rhs, g * rhs, b * rhs);
    }

    friend Color operator*(double lhs, const Color& rhs){
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
    double x, y, z;

    //default
    Point ( double s = 0 ) : x(s), y(s), z(s) {}

    // constructors
    Point ( double x, double y, double z ) : x(x), y(y), z(z) {}

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
    double x, y, z;

    //default
    Vector ( double s = 0 ) : x(s), y(s), z(s) {}

    // constructor, considering a vector from origin -> (x,y,z)
    Vector ( double xn, double yn, double zn, bool norm = false ) : x(xn), y(yn), z(zn) {
        if(norm) {
            double len = sqrt( x*x+y*y+z*z );
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
            double len = sqrt( x*x+y*y+z*z );
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

double distance ( const Point p, const Point q ) {
    double a = p.x - q.x;
    double b = p.y - q.y;
    double c = p.z - q.z;

    return sqrt(a*a + b*b + c*c);
}

double length ( const Vector v ) {
    return sqrt( v.x*v.x+v.y*v.y+v.z*v.z );
}

void normalize ( Vector& v ) {
    double len = length(v);

    if (len != 0.0f) {
        v.x = v.x / len;
        v.y = v.y / len;
        v.z = v.z / len;
    }
}

Vector cross ( const Vector v, const Vector u ) {
    return Vector( v.y*u.z - v.z*u.y , v.z*u.x - v.x*u.z , v.x*u.y - v.y*u.x );
}

double dot ( const Vector v , const Vector u ) {
    return ( v.x*u.x + v.y*u.y + v.z*u.z );
}

// normal should be normalized
Vector reflect ( Vector incoming, Vector normal ) {
    normalize( incoming );

    double twoMulDotProd = 2.0f * dot(incoming, normal);

    return Vector(incoming.x - twoMulDotProd * normal.x, incoming.y - twoMulDotProd * normal.y, incoming.z - twoMulDotProd * normal.z);
}

int indexMinElement ( std::vector<double> v ) {
    double minDist = *std::max_element(v.begin(), v.end());
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