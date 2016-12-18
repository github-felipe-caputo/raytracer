#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <vector>
#include <cmath>
#include "mathHelper.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

// This function will read an image and create a matrix (vector of vectors)
// with the pixel (Color) data of the image
std::vector<std::vector<Color> > loadImageFromFile(std::string filename){
    sf::Image image;
    if (!image.loadFromFile(filename)) {
        std::cerr << "Error: When loading file '" << filename << std::endl;
        exit(1);
    }

    sf::Vector2u size = image.getSize();
    int width = size.x;
    int height = size.y;

    std::vector<std::vector<Color> > texture(height, std::vector<Color>(width));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            sf::Color c = image.getPixel(10,10);
            texture[i][j] = Color(c.r,c.g,c.b);
        }
    }

    return texture;
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
Color planarCheckerTexture (std::vector<Point> vertices, Point p) {
    Color finalColor;

    int row, col;
    double checksize = 0.05;

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

// TODO still working on spherical texturing
Color sphericalCheckerTexture (Point center, double radius, Point p) {
    double phi = atan2(p.x, p.z);
    double theta = acos(p.y);
    if (phi < 0.0)
        phi += 2.0 * PI;

    double u = phi / (2.0 * PI);
    double v = 1.0 - theta / PI;




    return Color(0,0,0);
}

#endif
