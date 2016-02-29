#ifndef _LIGHTSOURCE_H
#define _LIGHTSOURCE_H

class LightSource {
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
};

#endif