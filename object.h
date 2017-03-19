#ifndef _OBJECT_H
#define _OBJECT_H

#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include "mathHelper.h"
#include "texture.h"

#include "triBoxOverlap.h"

class Object {
protected:
    // material
    Texture texture;

    // color, also ambient/diffuse for phong
    Color col;

    // other phong values
    Color specular;
    double ka, kd, ks, ke;

    // emmisive 'material' for area lights
    // if object is emissive, no need for any other color or value
    bool emissive = false;
    Color emissiveColor;

    // values for reflection and transmission
    double kr, kt;

    // value for refraction
    double nr;
public:
    // Object without solid color, called when creating textured object
    Object() {}

    Object(Color col) : col(col) {}

    Object(Texture texture) : texture(texture) {}

    virtual Point intersect (Ray ray) = 0;

    virtual std::vector<Point> samplePoints(int numSamples) = 0;

    virtual bool isInside (Voxel v) = 0;

    virtual Vector getNormal (Point p) = 0;

    // this function is to get a color in a specific point, if this object has
    // a texture
    virtual Color getColor (Point p) = 0;

    virtual void setPoints (std::vector<Point> vertices) = 0;

    virtual std::vector<Point> getPoints () = 0;

    Color getColor() {
        return col;
    }

    void setUpEmissionColor(Color ems) {
        emissive = true;
        emissiveColor = ems;
    }

    Color getEmissiveColor() {
        return emissiveColor;
    }

    bool isEmissive() {
        return emissive;
    }

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
    Color (*colorFromTexture)(Point, double, Point);// = NULL; initialization warning
public:

    // creating an object
    // if texture = true when creating, 'col' is ignored (function getColorFromTexture will be used instead)
    Sphere ( Point c, double r, Color col) : Object(col), c(c), r(r) {
        colorFromTexture = NULL;
    }

    Sphere ( Point c, double r, Color (*function)(Point, double, Point) ) : c(c), r(r) {
        colorFromTexture = function;
    }

    Sphere ( Point c, double r, Texture texture ) : Object(texture), c(c), r(r) {
    }

    Point intersect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        // a = 1 because direction of a ray is normalized
        //double A = 1;
        double B = 2.0 * (d.x * (o.x - c.x) + d.y * (o.y - c.y) + d.z * (o.z - c.z));
        double C = (o.x - c.x)*(o.x - c.x) + (o.y - c.y)*(o.y - c.y) + (o.z - c.z)*(o.z - c.z) - r*r;
        double w = 0;

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

    // Set point in sphere, definetly only one (center)
    void setPoints (std::vector<Point> vertices) {
        c = vertices[0];
    }

    std::vector<Point> getPoints () {
        return std::vector<Point>(1,c);
    }

    // returns a number of sample points on the surface of the object
    std::vector<Point> samplePoints(int numSamples) {
        std::vector<Point> samples;
        double n1, n2, n3;

        for (int i = 0; i < numSamples; ++i) {
            // numbers between -1 and 1
            n1 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/2.0)) - 1.0 ;
            n2 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/2.0)) - 1.0 ;
            n3 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/2.0)) - 1.0 ;

            Vector n(n1,n2,n3,true);
            samples.push_back( c + (r * Point(n.x,n.y,n.z)) );
        }

        return samples;
    }

    Vector getNormal (Point p) {
        Vector normal(c, p);
        normalize(normal);
        return normal;
    }

    Color getColor (Point p) {
        if (texture.isInitialized())
            return texture.getColorSphericalMapping(c,r,p);
        else if (*colorFromTexture != NULL)
            return (*colorFromTexture)(c,r,p);
        else
            return col;
    }
};

class Triangle : public Object {
    // Three vertices
    std::vector<Point> vertices;

    // Normal, based on vertices or not
    Vector normal;

    // this is a function pointer for a possible texture function,
    // it requires a vector of points (the vertices of the polygon) and a point
    // in the polygon as parameters, and returns the color of that point
    Color (*colorFromTexture)(std::vector<Point>, Point); // = NULL;
public:

    // Gets a vector of vertices, with 3 points, and creates a normal
    // for the triangle (vertices should be in clockwise order)
    Triangle (std::vector<Point> vert, Color col) : Object(col), vertices(vert) {
        normal = cross( Vector(vert[0],vert[1],true), Vector(vert[0],vert[2],true));
        colorFromTexture = NULL;
    }

    // Gets a vector of vertices, with 3 points, and the normal
    Triangle (std::vector<Point> vert, Vector norm, Color col) : Object(col), vertices(vert), normal(norm) {
        colorFromTexture = NULL;
    }

    // version with texture
    Triangle (std::vector<Point> vert, Color (*function)(std::vector<Point>, Point) ) : vertices(vert) {
        normal = cross( Vector(vert[0],vert[1],true), Vector(vert[0],vert[2],true));
        colorFromTexture = function;
    }

    // version with texture
    Triangle (std::vector<Point> vert, Vector norm, Color (*function)(std::vector<Point>, Point)) : vertices(vert), normal(norm) {
        colorFromTexture = function;
    }

    // Below, versions where you give the points not in a vector
    Triangle (Point p1, Point p2, Point p3, Color col) : Object(col) {
        vertices.push_back(p1);
        vertices.push_back(p2);
        vertices.push_back(p3);
        normal = cross( Vector(vertices[0],vertices[1],true), Vector(vertices[0],vertices[2],true));
    }

    // Gets a vector of vertices, with 3 points, and the normal
    Triangle (Point p1, Point p2, Point p3, Vector norm, Color col) : Object(col), normal(norm) {
        vertices.push_back(p1);
        vertices.push_back(p2);
        vertices.push_back(p3);
    }

    Triangle (Point p1, Point p2, Point p3, Color (*function)(std::vector<Point>, Point)) {
        vertices.push_back(p1);
        vertices.push_back(p2);
        vertices.push_back(p3);
        normal = cross( Vector(vertices[0],vertices[1],true), Vector(vertices[0],vertices[2],true));
        colorFromTexture = function;
    }

    // Gets a vector of vertices, with 3 points, and the normal
    Triangle (Point p1, Point p2, Point p3, Vector norm, Color (*function)(std::vector<Point>, Point)) : normal(norm) {
        vertices.push_back(p1);
        vertices.push_back(p2);
        vertices.push_back(p3);
        colorFromTexture = function;
    }

    // Code based on Tomas Akenine-Möller code at
    // http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/
    Point intersect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        double t, u, v, det, inv_det;
        Vector pvec, tvec, qvec;

        Vector edge1(vertices[0],vertices[1]);
        Vector edge2(vertices[0],vertices[2]);

        pvec = cross(d,edge2);
        det = dot(edge1, pvec);

        tvec = Vector(vertices[0], o);
        inv_det = 1.0 / det;

        qvec = cross(tvec,edge1);

        u = dot(tvec, pvec);
        if (u < 0.0 || u > det)
            return o;

        v = dot(d, qvec);
        if (v < 0.0 || u + v > det)
            return o;

        t = dot(edge2, qvec) * inv_det;
        u *= inv_det;
        v *= inv_det;

        if (t <= 0)
            return o;

        double wx = o.x + d.x * t;
        double wy = o.y + d.y * t;
        double wz = o.z + d.z * t;

        return Point(wx,wy,wz);
    }

    // returns a number of sample points on the surface of the object
    // TODO
    std::vector<Point> samplePoints(int numSamples) {
        std::vector<Point> samples;
        return samples;
    }

    void setPoints (std::vector<Point> vert) {
        vertices = vert;
        normal = cross( Vector(vert[0],vert[1],true), Vector(vert[0],vert[2],true));
    }

    std::vector<Point> getPoints () {
        return vertices;
    }

    // checks if this object is inside a voxel
    // returns true if even part of the object is inside of it
    bool isInside (Voxel v) {
        // Turn the data into something the function can understand
        Point center = v.getCenter();
        float boxcenter[] = {(float)center.x,(float)center.y,(float)center.z};

        Point halfSizes = v.getHalfLenghts();
        float boxhalfsize[] = {(float)halfSizes.x,(float)halfSizes.y,(float)halfSizes.z};

        float triverts[][3] = {{(float)vertices[0].x, (float)vertices[0].y, (float)vertices[0].z},
                               {(float)vertices[1].x, (float)vertices[1].y, (float)vertices[1].z},
                               {(float)vertices[2].x, (float)vertices[2].y, (float)vertices[2].z}};

        return (triBoxOverlap(boxcenter,boxhalfsize,triverts) == 1);
    }

    // For the triangle the normal is always the same
    Vector getNormal (Point p) {
        return normal;
    }

    Color getColor (Point p) {
        if (*colorFromTexture == NULL)
            return col;
        else
            return (*colorFromTexture)(vertices,p);
    }
};

class Rectangle : public Object {
    // Rectangle will have only 4 vertices
    // the vertices should be in clockwise position, i.e.
    //     2-----3
    //     |     |
    //     1-----4
    Point p1,p2,p3,p4;

    // normal, calculated based on vertices
    Vector n;

    // values for plane description
    double a,b,c,dist;

    // this is a function pointer for a possible texture function,
    // it requires a vector of points (the vertices of the polygon) and a point
    // in the polygon as parameters, and returns the color of that point
    Color (*colorFromTexture)(Point, Point, Point, Point, Point); // = NULL;
public:

    // creating an object
    // if texture = true when creating, 'col' is ignored (function getColorFromTexture will be used instead)
    // normal should be normalized before constructing the polygon
    Rectangle ( std::vector<Point> vert, Color col) : Object(col) {

        if (vert.size() != 4) {
            std::cerr << "Error: When creating a Rectangle object, need exactly 4 vertices, but " << vert.size() << " were used." << std::endl;
            exit(1);
        }

        p1 = vert[0];
        p2 = vert[1];
        p3 = vert[2];
        p4 = vert[3];

        canculatePlaneAndNormal();

        colorFromTexture = NULL;
    }

    // creating an object
    // instead of passing a color, pass a function for texture
    // normal should be normalized before constructing the polygon
    Rectangle ( std::vector<Point> vert, Color (*function)(Point, Point, Point, Point, Point) ) {

        if (vert.size() != 4) {
            std::cerr << "Error: When creating a Rectangle object, need exactly 4 vertices, but " << vert.size() << " were used." << std::endl;
            exit(1);
        }

        p1 = vert[0];
        p2 = vert[1];
        p3 = vert[2];
        p4 = vert[3];

        canculatePlaneAndNormal();

        colorFromTexture = function;
    }

    // Rectangle-ray intersection. First check intersection with plane, if it
    // happened then check intersection between the four points of the
    // recangle through dot products
    Point intersect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        double t = -(a*o.x + b*o.y + c*o.z + dist) / (a*d.x + b*d.y + c*d.z);

        // there was a intersection, let's check if it is between the rectangle boundaries
        if ( t > 0.0 ) {
            // actual intersection point
            double tx = o.x + d.x * t;
            double ty = o.y + d.y * t;
            double tz = o.z + d.z * t;
            Point intersectionPoint(tx, ty, tz);

            Vector v1(p1,p2,true);
            Vector v3(p3,p4,true);
            Vector v4(p1,intersectionPoint,true);
            Vector v5(p3,intersectionPoint,true);

            Vector v1_a(p2,p3,true);
            Vector v3_a(p4,p1,true);
            Vector v4_a(p2,intersectionPoint,true);
            Vector v5_a(p4,intersectionPoint,true);

            if (dot(v1,v4) >= 0.0 && dot(v3,v5) >= 0.0
                && dot(v1_a,v4_a) >= 0.0 && dot(v3_a,v5_a) >= 0.0 ) {
                return intersectionPoint;
            }
        }

        return o;
    }

    // Returns a number of sample points on the surface of the object
    std::vector<Point> samplePoints(int numSamples) {
        std::vector<Point> samples;
        int samplesBySide = numSamples / 2;

        Vector v1 = Vector(p1,p2) / samplesBySide;
        Vector v2 = Vector(p1,p4) / samplesBySide;

        for(int i = 0; i < samplesBySide; ++i) {
            for(int j = 0; j < samplesBySide; ++j) {
                double startx = p1.x + i * v1.x + j * v2.x;
                double starty = p1.y + i * v1.y + j * v2.y;
                double startz = p1.z + i * v1.z + j * v2.z;
                Point start(startx,starty,startz);

                // numbers between 0 and 1
                double u = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX));
                double v = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX));

                double samplex = start.x + (u * v1.x) + (v * v2.x);
                double sampley = start.y + (u * v1.y) + (v * v2.y);
                double samplez = start.z + (u * v1.z) + (v * v2.z);

                Point samplePoint(samplex + 0.0 * n.x,
                                  sampley + 0.0 * n.y,
                                  samplez + 0.0 * n.z);

                samples.push_back( samplePoint );
            }
        }

        return samples;
    }

    void setPoints (std::vector<Point> vertices) {
        p1 = vertices[0];
        p2 = vertices[1];
        p3 = vertices[2];
        p4 = vertices[3];
        canculatePlaneAndNormal();
    }

    std::vector<Point> getPoints () {
        std::vector<Point> vertices(4);
        vertices[0] = p1;
        vertices[1] = p2;
        vertices[2] = p3;
        vertices[3] = p4;
        return vertices;
    }

    // checks if this object is inside a voxel
    // returns true if even part of the object is inside of it
    bool isInside (Voxel v) {
        // Cheat: create two triangles out of the rectangle points, and check
        // those triangles for intersections!
        Triangle t1(p1,p2,p3,NULL);
        Triangle t2(p1,p3,p4,NULL);

        return (t1.isInside(v) || t2.isInside(v));
    }

    Vector getNormal (Point p) {
        return n;
    }

    Color getColor (Point p) {
        if (*colorFromTexture == NULL)
            return col;
        else
            return (*colorFromTexture)(p1,p2,p3,p4,p);
    }

    void canculatePlaneAndNormal() {
        a = p1.y*(p2.z-p3.z) + p2.y*(p3.z-p1.z) + p3.y*(p1.z-p2.z);
        b = p1.z*(p2.x-p3.x) + p2.z*(p3.x-p1.x) + p3.z*(p1.x-p2.x);
        c = p1.x*(p2.y-p3.y) + p2.x*(p3.y-p1.y) + p3.x*(p1.y-p2.y);
        dist = -p1.x*(p2.y*p3.z-p3.y*p2.z) - p2.x*(p3.y*p1.z - p1.y*p3.z) - p3.x*(p1.y*p2.z - p2.y*p1.z);

        n = cross( Vector(p1,p4) , Vector(p1,p2) );
        normalize(n);
    }
};

#endif
