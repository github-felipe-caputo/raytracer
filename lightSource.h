#ifndef _LIGHTSOURCE_H
#define _LIGHTSOURCE_H

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
    virtual float getAttenuation(Point p) = 0;
};

class PointLight : public LightSource {
public:
    PointLight( Point position, Color color ) : LightSource(position, color) {} 

    // Point light can always be reached
    bool reaches(Point p) {
        return true;
    }

    // Point light attenuation == 1, none
    float getAttenuation (Point p) {
        return 1;
    }
};

class SpotLight : public LightSource {
    Vector dir;
    float angle;
    float aExp; // attenuation exponent;

public:
    SpotLight( Point position, Color color, Vector dir, float angle, float aExp ) : LightSource(position, color), dir(dir), angle(angle), aExp(aExp) {} 

    // SpotLight might not be reached depending on the angle and direction
    bool reaches(Point p) {
        Vector vObj(position, p, true);
        float cosa = dot(vObj, dir);

        return (cosa >= std::cos(angle * PI / 180.0f));
    }

    float getAttenuation(Point p) {
        Vector vObj(position, p, true);
        return std::pow ( dot(vObj, dir), aExp );
    }
};

#endif