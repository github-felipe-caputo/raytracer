#ifndef _CAMERA_H
#define _CAMERA_H

#define RAY_CENTER 0
#define RAY_GRID   1

#define RAY_TRACER   2
#define RAY_MARCHING 3

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

    // max depth, the bounces of the ray
    int MAX_DEPTH;

    // If we are going ray tracking or ray marching
    int rayType;

    // if the ray is going on the center of the pixel, or in a grid
    int gridOrCenter;

    // for ray marching
    int SAMPLE_NUM;

public:

    /**
     * NOTE: I'm not using the up vector yet, just assuming it's positive y
     */

    // rayType = if we are doing ray tracing or ray marching
    // ray marching here was implemented so far only for volumetric lighthing,
    // so some other values will need to be set up before using it (ka and ks for instance)
    Camera(Point pos, Vector look, Vector up, int imH, int imW, double viewH, double viewW, int rayType, int depthOrSamples, int gridOrCenter) :
        position(pos), lookAt(look), up(up), imageHeight(imH), imageWidth(imW), viewPlaneHeigth(viewH), viewPlaneWidth(viewW), rayType(rayType), gridOrCenter(gridOrCenter) {

        // each pixel
        unitsHigh = viewH/imH;
        unitsWidth = viewW/imW;

        //normalize lookat
        normalize(lookAt);

        if (rayType == RAY_TRACER) {
            MAX_DEPTH = depthOrSamples;
        }
        else if (rayType == RAY_MARCHING) {
            SAMPLE_NUM = depthOrSamples;
        }
        else {
            std::cerr << "Error: When creating a camera object '" << rayType << "' is an invalid value for 'rayType', use RAY_TRACER or RAY_MARCHING." << std::endl;
            exit(1);
        }

        if ( gridOrCenter != RAY_CENTER && gridOrCenter != RAY_GRID ) {
            std::cerr << "Error: When creating a camera object '" << gridOrCenter << "' is an invalid value for 'gridOrCenter', use RAY_CENTER or RAY_GRID." << std::endl;
            exit(1);
        }
    }

    std::vector<Color> render (World world) {
        if (rayType == RAY_TRACER)
            return renderRayTracer (world);
        else
            return renderRayMarch (world);
    }


    std::vector<Color> renderRayTracer (World world) {
        // "first pixel is at
        double firstPlanex = -viewPlaneWidth/2;
        double firstPlaney = viewPlaneHeigth/2;

        // ray direction
        double dx,dy,dz;

        // Result color of a ray
        std::vector<Color> colorMap;

        // creating rays
        // for now I'm assuming position of camera is origin, focalLength = -0.5f (not using lookAt value yet);
        double focalLength = -0.5;

        // this loop is going like
        // consider origin at top left
        // fix column
        //    go through the rows in the column
        // then go to next column

        if(gridOrCenter == RAY_CENTER)
        {
            for(int i = 0; i < imageWidth; ++i) {
                for(int j = 0; j < imageHeight; ++j) {
                    dx = firstPlanex + (unitsWidth/2.0) + i * unitsWidth;
                    dy = firstPlaney - (unitsHigh/2.0) - j * unitsHigh;
                    dz = focalLength;

                    // vector direction, normalize
                    Vector dir(dx,dy,dz, true);

                    // ray
                    Ray ray(position, dir);

                    // spawn rays into the world to get the color
                    colorMap.push_back( world.spawn( ray , MAX_DEPTH ) );
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
                            dx = firstPlanex + (unitsWidth * (1/4 * a)) + i * unitsWidth;
                            dy = firstPlaney - (unitsHigh * (1/4 * b)) - j * unitsHigh;
                            dz = focalLength;

                            // vector direction, normalize
                            Vector dir(dx,dy,dz, true);

                            // ray
                            Ray ray(position, dir);

                            // Color average
                            average += world.spawn( ray , MAX_DEPTH );
                        }
                    }

                    // get final color
                    average = (average / 9.0);
                    colorMap.push_back( average );
                }
            }
        }

        // will return a vector with imageWidth * imageHeight values, use it to paint the canvas
        return colorMap;
    }


    std::vector<Color> renderRayMarch (World world) {
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

        if(gridOrCenter == RAY_CENTER)
        {
            for(int i = 0; i < imageWidth; ++i) {
                for(int j = 0; j < imageHeight; ++j) {
                    dx = firstPlanex + (unitsWidth/2.0) + i * unitsWidth;
                    dy = firstPlaney - (unitsHigh/2.0) - j * unitsHigh;
                    dz = focalLength;

                    // vector direction, normalize
                    Vector dir(dx,dy,dz, true);

                    // ray
                    Ray ray(position, dir);

                    // spawn rays into the world to get the color
                    colorMap.push_back( world.spawnRayMarch( ray , SAMPLE_NUM ) );
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
                            dx = firstPlanex + (unitsWidth * (1.0/4.0 * a)) + i * unitsWidth;
                            dy = firstPlaney - (unitsHigh * (1.0/4.0 * b)) - j * unitsHigh;
                            dz = focalLength;

                            // vector direction, normalize
                            Vector dir(dx,dy,dz, true);

                            // ray
                            Ray ray(position, dir);

                            // Color average
                            average += world.spawnRayMarch( ray , SAMPLE_NUM );
                        }
                    }

                    // get final color
                    average = (average / 9.0);
                    colorMap.push_back( average );
                }
            }
        }

        // will return a vector with imageWidth * imageHeight values, use it to paint the canvas
        return colorMap;
    }

};

#endif
