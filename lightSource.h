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

    // Ray Marching
    virtual std::vector<Point> intersect ( Ray ray ) = 0;
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

    std::vector<Point> intersect ( Ray ray ) {
        // does nothing so far
        std::vector<Point> intersections;
        return intersections;
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

        return (cosa >= std::cos(angle * PI / 180.0));
    }

    double getAttenuation (Point p) {
        Vector vObj(position, p, true);
        return std::pow ( dot(vObj, dir), aExp );
    }

    // This intersection will be used for the ray marching,
    // based on http://www.geometrictools.com/Documentation/IntersectionLineCone.pdf
    std::vector<Point> intersect ( Ray ray ) {
        std::vector<Point> intersections;
        Point origin = ray.getOrigin();
        Vector direction = ray.getDirection();
        normalize(direction);

        // here we will turn some vectors in matrixes for calculations

        double deltaVals[] = {origin.x-position.x,origin.y-position.y,origin.z-position.z};
        Matrix delta(3,1,deltaVals);

        Matrix d(dir);
        Matrix u(direction);

        Matrix m = d * d.transpose() - std::pow(std::cos(angle * PI / 180.0),2) * indentityMatrix();
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

            // there was a intersection
            if (w1 > 0.0) {
                Point inter(origin.x + direction.x * w1, origin.y + direction.y * w1, origin.z + direction.z * w1);
                // if we are hitting the actual cone
                if (dot( dir , Vector(position, inter, true) ) >= 0)
                    intersections.push_back( inter );
            }

        } else if (c1c1minusc0c2 > 0) {
            w1 = - (c1 - sqrt(c1c1minusc0c2)) / c2;
            w2 = - (c1 + sqrt(c1c1minusc0c2)) / c2;

            if (w1 > 0.0) {
                Point inter1(origin.x + direction.x * w1, origin.y + direction.y * w1, origin.z + direction.z * w1);
                if ( dot( dir , Vector(position, inter1, true) ) >= 0 )
                    intersections.push_back( inter1 );
            }

            if (w2 > 0.0) {
                Point inter2(origin.x + direction.x * w2, origin.y + direction.y * w2, origin.z + direction.z * w2);
                if ( dot( dir , Vector(position, inter2, true) ) >= 0 )
                    intersections.push_back( inter2 );
            }
        }

        return intersections;
    }
};

#endif
