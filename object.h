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

    // texture set? Always starts as false if nothing is shown
    bool texture;

public:
    Object(Color col, bool texture) : col(col), texture(texture) {}
    
    virtual Point intersect (Ray ray) = 0;

    virtual Vector getNormal (Point p) = 0;
    
    // this function is to get a color in a specific point, if this object has
    // a texture
    Color getColor (Point p) {
        if (texture == true)
            return getColorFromTexture(p);
        else
            return col;
    }

    // kd + ks < 1 YOU PAY ATTENTION JESUS
    void setUpPhong (Color spec, double newka, double newkd, double newks, double newke) { 
        specular = spec;
        ka = newka;
        kd = newkd;
        ks = newks;
        ke = newke; 
    }

    // for now just set value to true
    void setUpTexture () {
        texture = true;
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

    virtual Color getColorFromTexture (Point p) = 0;

};

class Sphere : public Object {
    // Center and radius
    Point c;
    double r;

public:

    // creating an object
    // if texture = true when creating, 'col' is ignored (function getColorFromTexture will be used instead)
    Sphere ( Point c, double r, Color col, bool texture = false) : Object(col,texture), c(c), r(r) {}

    Point intersect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        // a = 1 because direction of a ray is normalized
        //double A = 1;
        double B = 2.0f * (d.x * (o.x - c.x) + d.y * (o.y - c.y) + d.z * (o.z - c.z));
        double C = (o.x - c.x)*(o.x - c.x) + (o.y - c.y)*(o.y - c.y) + (o.z - c.z)*(o.z - c.z) - r*r;
        double w;

        double BBminus4C = B*B - 4.0f*C;

        if (BBminus4C < 0) 
        {
            return o; // will return the point origin if there is no intersection
        }
        else if (BBminus4C == 0) 
        {
            w = (-B + sqrt(B*B - 4.0f*C)) / 2.0f;
            return Point(o.x + d.x * w, o.y + d.y * w, o.z + d.z * w);
        } 
        else 
        {
            double w1 = (-B + sqrt(B*B - 4.0f*C)) / 2.0f;
            double w2 = (-B - sqrt(B*B - 4.0f*C)) / 2.0f;

            if (w1 > 0 && w1 <= w2) // w1 is positive and smaller than or equal to w2
                w = w1;
            else if (w2 > 0) // w2 is positive and either smaller than w1 or w1 is negative
                w = w2;      

            return Point(o.x + d.x * w, o.y + d.y * w, o.z + d.z * w);
        } 
        
    } 

    Vector getNormal (Point p) {
        Vector normal(c, p);
        normalize(normal);
        return normal;
    }

    // not going to use it yet, just testing for compiling
    Color getColorFromTexture (Point p) {
        return col;
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

public:

    // creating an object
    // if texture = true when creating, 'col' is ignored (function getColorFromTexture will be used instead)
    // normal should be normalized before constructing the polygon
    Polygon ( std::vector<Point> vert, Vector n, Color col, bool texture = false) : Object(col,texture), vertices(vert), n(n) {

        // for now I'm assuming all the vertices are on the same plane, and that place
        // has a normal (0,1,0), and all the points are on a plane parallel to the x and z plane
        // so f will always be y of any of the vertices (they are always the same)
        f = std::abs( vert[0].y );
    }

    Point intersect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        // ray-plane intersection
        double w = -(n.x*o.x + n.y*o.y + n.z*o.z + f) / (n.x*d.x + n.y*d.y + n.z*d.z);

        // there was a intersection
        if ( w > 0.0f ) {
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

    Vector getNormal (Point p) {
        return n;
    }

    // let's assume for now this will always be used for a floor that always coincides
    // with the x and z plane (y is constant) 
    //
    // this function will work also on the assumption that the points on the vertices list is like so:
    //
    //     1-----2
    //     |     |
    //     0-----3
    //
    // looking down on y
    Color getColorFromTexture (Point p) {
        Color finalColor;

        int row, col;
        double checksize = 0.1f;

        // need to normalize point between -1 and 1
        double zn = (2 * (p.z - vertices[1].z) / (vertices[0].z - vertices[1].z)) - 1;
        double xn = (2 * (p.x - vertices[1].x) / (vertices[2].x - vertices[1].x)) - 1;

        double u = (zn + 1.0f) / 2.0f;
        double v = (xn + 1.0f) / 2.0f;

        // find row and col
        for (int i = 0; i < (1/checksize); ++i){
            if (i*checksize < v && v < (i+1)*checksize) {
                row = i;
            }
            if (i*checksize < u && u < (i+1)*checksize) {
                col = i;
            }
        }

        // check even or odds to know the color
        if (row % 2 && col % 2) {
            finalColor = Color(1,0,0);
        } else if (row % 2 && !(col % 2)) {
            finalColor = Color(1,1,0);
        } else if (!(row % 2) && col % 2) {
            finalColor = Color(1,1,0);
        } else {
            finalColor = Color(1,0,0);
        }

        return finalColor;
    }

};


#endif