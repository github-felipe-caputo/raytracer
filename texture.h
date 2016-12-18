#ifndef _TEXTUREOLD_H
#define _TEXTUREOLD_H

#include <vector>
#include <cmath>
#include "mathHelper.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

class Texture {
    int height, width;
    std::vector<Color > texture;

public:
    // default constructor
    Texture() {}

    // This constructor will read an image and create a matrix (vector of vectors)
    // with the pixel (Color) data of the image
    Texture(std::string filename) {
        sf::Image image;
        if (!image.loadFromFile(filename)) {
            std::cerr << "Error: When loading file '" << filename << std::endl;
            exit(1);
        }

        sf::Vector2u size = image.getSize();
        width = size.x;
        height = size.y;

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                sf::Color c = image.getPixel(10,10);
                texture.push_back( Color(c.r,c.g,c.b) );
            }
        }

    }

    Color getColorSphericalMapping(Point center, double radius, Point intersection) {
        double phi = atan2(intersection.x, intersection.z);
        double theta = acos(intersection.y);
        if (phi < 0.0)
            phi += 2.0 * PI;

        double u = phi / (2.0 * PI);
        double v = 1.0 - theta / PI;

        // TODO: get the actual expected color from texture

        return Color(0,0,0);
    }
};

#endif
