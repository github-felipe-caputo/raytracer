#ifndef _CAMERA_H
#define _CAMERA_H

#define RAY_CENTER 0
#define RAY_GRID   1

#include <vector>
#include "mathHelper.h"
#include "world.h"

class Camera {
    // camera'ss position
    Point position;

    // camera's lookat
    Vector lookAt;

    // camera's up vector
    Vector up;

    //
    // values to render the world
    //

    // pixels
    int imageHeight, imageWidth;

    // keeping the aspect ratio of the window pixels
    double viewPlaneHeigth, viewPlaneWidth;

    // each pixel
    double unitsHigh, unitsWidth;

    // How we are spawing the rays
    int rayType;

public:

    /**
     * NOTE: I'm not using the up vector yet, just assuming it's positive y
     */

    Camera(Point pos, Vector look, Vector up, int imH, int imW, double viewH, double viewW, int rayType) : 
        position(pos), lookAt(look), up(up), imageHeight(imH), imageWidth(imW), viewPlaneHeigth(viewH), viewPlaneWidth(viewW), rayType(rayType) {
        
        // each pixel
        unitsHigh = viewH/imH;
        unitsWidth = viewW/imW;

        //normalize lookat
        normalize(lookAt);
    }

    std::vector<Color> render (World world) {
        // "first pixel is at
        double firstPlanex = -viewPlaneWidth/2;
        double firstPlaney = viewPlaneHeigth/2;

        // ray direction
        double dx,dy,dz;

        // Result color of a ray
        std::vector<Color> colorMap;

        // creating rays
        // for now I'm assuming position of camera is origin, focalLength = -0.5f (not using lookAt value yet);
        double focalLength = -0.5f;

        // this loop is going like
        // consider origin at top left
        // fix column
        //    go through the rows in the column
        // then go to next column

        if(rayType == RAY_CENTER) 
        {
            for(int i = 0; i < imageWidth; ++i) {
                for(int j = 0; j < imageHeight; ++j) {
                    dx = firstPlanex + (unitsWidth/2.0f) + i * unitsWidth;
                    dy = firstPlaney - (unitsHigh/2.0f) - j * unitsHigh;
                    dz = focalLength;
                    
                    // vector direction, normalize
                    Vector dir(dx,dy,dz, true);

                    // ray
                    Ray ray(position, dir);

                    // spawn rays into the world to get the color
                    colorMap.push_back( world.spawn( ray ) );
                }
            }
        } 
        else 
        {
            for(int i = 0; i < imageWidth; ++i) {
                for(int j = 0; j < imageHeight; ++j) {

                    Color average;
                    /// lets make a grid of 9 rays
                    for (int a = 1; a < 4; ++a) {
                        for (int b = 1; b < 4; ++b) {
                            dx = firstPlanex + (unitsWidth/(2.5f * a)) + i * unitsWidth;
                            dy = firstPlaney - (unitsHigh/(2.5f * b)) - j * unitsHigh;
                            dz = focalLength;

                            // vector direction, normalize
                            Vector dir(dx,dy,dz, true);

                            // ray
                            Ray ray(position, dir);

                            // Color average
                            average += world.spawn( ray );
                        }
                    }

                    // get final color
                    average = (average / 9.0f);
                    colorMap.push_back( average );
                }
            }
        }

        // will return a vector with imageWidth * imageHeight values, use it to paint the canvas
        return colorMap;
    }

};

#endif