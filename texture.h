#ifndef _TEXTUREOLD_H
#define _TEXTUREOLD_H

#include <vector>
#include <cmath>
#include "mathHelper.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

class Texture {
    // sentinel value to see if texture was setup
    bool initialized;

    int height, width;

    std::vector<Color > texture;

public:
    // default constructor
    Texture() {
        initialized = false;
    }

    // This constructor will read an image and create a 2D array (represented in 1D)
    // with the pixel (Color) data of the image
    Texture(std::string filename) {
        initialized = true;
        sf::Image image;
        if (!image.loadFromFile(filename)) {
            std::cerr << "Error: When loading file '" << filename << std::endl;
            exit(1);
        }

        sf::Vector2u size = image.getSize();
        width = size.x;
        height = size.y;

        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {
                sf::Color c = image.getPixel(i,j);
                texture.push_back( Color( double(c.r)/255.0,
                                          double(c.g)/255.0,
                                          double(c.b)/255.0 ) );
            }
        }

    }

    bool isInitialized() {
        return initialized;
    }

    // Assumes a 2:1 aspect ratio texture
    Color getColorSphericalMapping(Point c, double r, Point p) {
        Vector local(p.x-c.x,p.y-c.y,p.z-c.z);
        normalize(local);

        double u = 0.5 + atan2(local.x,local.z) / (2.0 * PI);
        double v = 0.5 - asin(local.y) / PI;

        int row = (int) (height - 1) * v;
        int col = (int) (width - 1) * u;

        return texture[row * width + col];
    }
};

#endif
