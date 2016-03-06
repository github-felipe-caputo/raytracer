#ifndef _CAMERA_H
#define _CAMERA_H

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
    float viewPlaneHeigth, viewPlaneWidth;

    // each pixel
    float unitsHigh, unitsWidth;

public:

    /**
     * NOTE: I'm not using the up vector yet, just assuming it's positive y
     */

    Camera(Point pos, Vector look, Vector up, int imH, int imW, float viewH, float viewW) : 
        position(pos), lookAt(look), up(up), imageHeight(imH), imageWidth(imW), viewPlaneHeigth(viewH), viewPlaneWidth(viewW) {
        
        // each pixel
        unitsHigh = viewH/imH;
        unitsWidth = viewW/imW;

        //normalize lookat
        normalize(lookAt);
    }

    std::vector<Color> render (World world) {
        // "first pixel is at
        float firstPlanex = -viewPlaneWidth/2;
        float firstPlaney = viewPlaneHeigth/2;

        // ray direction
        float dx,dy,dz;

        // Result color of a ray
        std::vector<Color> colorMap;

        // creating rays
        // for now I'm assuming position of camera is origin, focalLength = -0.5f (not using lookAt value yet);
        float focalLength = -0.5f;

        // this loop is going like
        // consider origin at top left
        // fix column
        //    go through the rows in the column
        // then go to next column

        for(int i = 0; i < imageWidth; ++i) {
            for(int j = 0; j < imageHeight; ++j) {
                dx = firstPlanex + (unitsWidth/2) + i * unitsWidth;
                dy = firstPlaney - (unitsHigh/2) - j * unitsHigh;
                dz = focalLength;
                
                // vector direction, normalize
                Vector dir(dx,dy,dz, true);

                // ray
                Ray ray(position, dir);

                // spawn rays into the world to get the color
                colorMap.push_back( world.spawn( ray ) );
            }
        }

        // will return a vector with imageWidth * imageHeight values, use it to paint the canvas
        return colorMap;
    }

};

#endif