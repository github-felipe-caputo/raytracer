#ifndef _PROCEDURALTEXTURE_H
#define _PROCEDURALTEXTURE_H

#include <vector>
#include <cmath>
#include "mathHelper.h"

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

    double u = (zn + 1.0) / 2.0;
    double v = (xn + 1.0) / 2.0;

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


#endif
