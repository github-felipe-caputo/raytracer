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

    std::vector<Point> intersection ( Ray ray ) {
        std::vector<Point> intersections;
        Point o = ray.getOrigin();
        Vector d = ray.getDirection();
        normalize(d);

        // 'vector' delta
        Point delta(o.x - position.x, o.y - position.y, o.z - position.z);

        // matrix m is a identity matrix with these values
        Point M(dir.x * dir.x - std::pow(std::cos(angle * PI / 180.0f),2),
                dir.y * dir.y - std::pow(std::cos(angle * PI / 180.0f),2),
                dir.z * dir.z - std::pow(std::cos(angle * PI / 180.0f),2));

        Point M1(dir.x * dir.x - std::pow(std::cos(angle * PI / 180.0f),2),dir.x * dir.y, dir.x * dir.z);
        Point M2(dir.x * dir.y,dir.y * dir.y - std::pow(std::cos(angle * PI / 180.0f),2), dir.y * dir.z);
        Point M3(dir.x * dir.z,dir.y * dir.z, dir.z * dir.z - std::pow(std::cos(angle * PI / 180.0f),2));


        std::cout << d.y * d.y * M.y << std::endl;


        double c2 = ( d.x * d.x * M.x + d.y * d.y * M.y + d.z * d.z * M.z );
        double c1 = ( d.x * delta.x * M.x + d.y * delta.y * M.y + d.z * delta.z * M.z );
        double c0 = ( delta.x * delta.x * M.x + delta.y * delta.y * M.y + delta.z * delta.z * M.z );

        double w1, w2;

        double c1c1minusc0c2 = c1 * c1 - c0 * c2;

        std::cout << c2 << std::endl;

        if (c1c1minusc0c2 == 0) {
            w1 = - c1 / c2;
            intersections.push_back( Point(o.x + d.x * w1, o.y + d.y * w1, o.z + d.z * w1) );
        } else if (c1c1minusc0c2 > 0) {
            w1 = - (c1 + sqrt(c1c1minusc0c2)) / c2;
            w2 = - (c1 - sqrt(c1c1minusc0c2)) / c2;

            intersections.push_back( Point(o.x + d.x * w1, o.y + d.y * w1, o.z + d.z * w1) );
            intersections.push_back( Point(o.x + d.x * w2, o.y + d.y * w2, o.z + d.z * w2) );
        } 

        return intersections;
    }
};

#endif