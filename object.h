#ifndef _OBJECT_H
#define _OBJECT_H

#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include "mathHelper.h"
#include "texture.h"

#include "triBoxOverlap.h"

#define NUM_POINT_SAMPLES_ON_LIGHT 20

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

    virtual std::vector<Point> samplePoints() = 0;

    virtual bool isInside (Voxel v) = 0;

    virtual Vector getNormal (Point p) = 0;

    // this function is to get a color in a specific point, if this object has
    // a texture
    virtual Color getColor (Point p) = 0;

    Color getColor() {
        return col;
    }

    void setUpEmissionColor(Color ems) {
        emissive = true;
        emissiveColor = ems;
    }

    Color getEmissiveColor() {
        return emissive;
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

    // returns a number of sample points on the surface of the object
    std::vector<Point> samplePoints() {
        std::vector<Point> samples;
        double n1, n2, n3;

        for (int i = 0; i < NUM_POINT_SAMPLES_ON_LIGHT; ++i) {
            // numbers between -1 and 1
            n1 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/2.0)) - 1.0 ;
            n2 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/2.0)) - 1.0 ;
            n3 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/2.0)) - 1.0 ;

            Vector n(n1,n2,n3,true);

            samples.push_back( intersect( Ray(c,n) ) );
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
    Color (*colorFromTexture)(std::vector<Point>, Point); // = NULL;
public:

    // creating an object
    // if texture = true when creating, 'col' is ignored (function getColorFromTexture will be used instead)
    // normal should be normalized before constructing the polygon
    Polygon ( std::vector<Point> vert, Vector n, Color col) : Object(col), vertices(vert), n(n) {

        // for now I'm assuming all the vertices are on the same plane, and that place
        // has a normal (0,1,0), and all the points are on a plane parallel to the x and z plane
        // so f will always be y of any of the vertices (they are always the same)
        f = std::abs( vert[0].y );

        colorFromTexture = NULL;
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

    // NOTE: this intersection is not taking into account the normal yet,
    // in other words, it will return an intersection even if the triangle is
    // "backwards"
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

    // returns a number of sample points on the surface of the object
    // TODO
    std::vector<Point> samplePoints() {
        std::vector<Point> samples;
        return samples;
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
    //
    Point intersect (Ray ray) {
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        double EPSILON = 0.000001;
        double t, u, v, det, inv_det;
        Vector pvec, tvec, qvec;

        Vector edge1(vertices[0],vertices[1]);
        Vector edge2(vertices[0],vertices[2]);

        pvec = cross(d,edge2);
        det = dot(edge1, pvec);

        tvec = Vector(vertices[0], o);
        inv_det = 1.0 / det;

        qvec = cross(tvec,edge1);

        if (det < EPSILON)
            return o;

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
    std::vector<Point> samplePoints() {
        std::vector<Point> samples;
        return samples;
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

#endif
