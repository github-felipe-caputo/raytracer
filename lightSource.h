#ifndef _LIGHTSOURCE_H
#define _LIGHTSOURCE_H

#include <vector>
#include "mathHelper.h"

class LightSource {
protected:
    Point position;
    Color color;

public:

    LightSource ( Point position, Color color ) : position(position), color(color) {}

    Point getPos () {
        return position;
    } 

    Color getColor () {
        return color;
    } 

    virtual bool reaches(Point p) = 0;
    virtual double getAttenuation(Point p) = 0;
};

class PointLight : public LightSource {
public:
    PointLight( Point position, Color color ) : LightSource(position, color) {} 

    // Point light can always be reached
    bool reaches (Point p) {
        return true;
    }

    // Point light attenuation = 1, none
    double getAttenuation (Point p) {
        return 1.0f;
    }
};

class SpotLight : public LightSource {
    Vector dir;
    double angle;
    double aExp; // attenuation exponent;

public:
    SpotLight( Point position, Color color, Vector dir, double angle, double aExp = 0 ) : LightSource(position, color), dir(dir), angle(angle), aExp(aExp) {} 

    // SpotLight might not be reached depending on the angle and direction
    bool reaches (Point p) {
        Vector vObj(position, p, true);
        double cosa = dot(vObj, dir);

        return (cosa >= std::cos(angle * PI / 180.0f));
    }

    double getAttenuation (Point p) {
        Vector vObj(position, p, true);
        return std::pow ( dot(vObj, dir), aExp );
    }

    // This intersection will be used for the ray marching,
    // based on http://www.geometrictools.com/Documentation/IntersectionLineCone.pdf
    std::vector<Point> intersection ( Ray ray ) {
        std::vector<Point> intersections;
        Point origin = ray.getOrigin();
        Vector direction = ray.getDirection();
        normalize(direction);

        // here we will turn some vectors in matrixes for calculations

        double deltaVals[] = {origin.x-position.x,origin.y-position.y,origin.z-position.z};
        Matrix delta(3,1,deltaVals);

        Matrix d(dir);
        Matrix u(direction);

        Matrix m = d * d.transpose() - std::pow(std::cos(angle * PI / 180.0f),2) * indentityMatrix();
        Matrix mc2 = u.transpose() * m * u; // matrix with one row and one col
        Matrix mc1 = u.transpose() * m * delta; // matrix with one row and one col
        Matrix mc0 = delta.transpose() * m * delta; // matrix with one row and one col

        double w1, w2;
        double c0 = mc0[0];
        double c1 = mc1[0];
        double c2 = mc2[0];

        double c1c1minusc0c2 = c1 * c1 - c0 * c2;

        if ( c1c1minusc0c2 == 0 ) {
            w1 = - c1 / c2;
            intersections.push_back( Point(origin.x + direction.x * w1, origin.y + direction.y * w1, origin.z + direction.z * w1) );
        } else if (c1c1minusc0c2 > 0) {
            w1 = - (c1 + sqrt(c1c1minusc0c2)) / c2;
            w2 = - (c1 - sqrt(c1c1minusc0c2)) / c2;

            intersections.push_back( Point(origin.x + direction.x * w1, origin.y + direction.y * w1, origin.z + direction.z * w1) );
            intersections.push_back( Point(origin.x + direction.x * w2, origin.y + direction.y * w2, origin.z + direction.z * w2) );
        } 

        return intersections;
    }
};

#endif