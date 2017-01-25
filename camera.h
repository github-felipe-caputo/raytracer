#ifndef _CAMERA_H
#define _CAMERA_H

#define RAY_TRACER   2
#define RAY_MARCHING 3

#include <vector>
#include "mathHelper.h"
#include "world.h"

#include <future>
#include <thread>

class Camera {
    // camera'ss position
    Point position;

    // camera's lookat
    Point lookAt;

    // camera's up vector
    Vector up;

    // the view-plane distance, fixed at 0.5
    double focalLength = 0.5;

    // view corrdinate system
    Vector u,v,w;

    //
    // values to render the world
    //

    // pixels
    int imageHeight, imageWidth;

    // keeping the aspect ratio of the window pixels
    double viewPlaneHeigth, viewPlaneWidth;

    // each pixel
    double unitsHigh, unitsWidth;

    // 'first pixels' where the view will stay
    double firstPixelx, firstPixely;

    // max depth, the bounces of the ray
    int MAX_DEPTH;

    // If we are going ray tracking or ray marching
    int rayType;

    // number of rays we will use per pixel, total is raysPerPixel * raysPerPixel
    int raysPerPixel;

    // for ray marching
    int SAMPLE_NUM;

    // This function is given the world and the pixel, it will return the color
    // of that pixel. In other words i ranges from [0,imageWidth] and
    // j ranges from [0,imageHeight]
    Color getColorInPixel(World world, int i, int j){
        // ray direction
        double dx,dy,dz;

        // Color average
        Color average;

        // size of ray grid in the pixel
        int gridSize = raysPerPixel + 1; // i.e.: raysPerPixel = 3 -> gridSize = 4 -> from 1 to 4 both dimentions, i.e., 3 x 3 = 9 rays
        double range = static_cast <double> (1.0 / gridSize);

        for (int a = 1; a < gridSize; ++a) {
            for (int b = 1; b < gridSize; ++b) {
                dx = firstPixelx + (unitsWidth * (range * a)) + i * unitsWidth;
                dy = firstPixely - (unitsHigh * (range * b)) - j * unitsHigh;
                dz = focalLength;

                Vector dir = dx*u + dy*v - dz*w;
                normalize(dir);

                // ray
                Ray ray(position, dir);

                // Color average
                //if (rayType == RAY_TRACER)
                    average += world.spawn( ray , MAX_DEPTH );
                //else
                //    average += world.spawnRayMarch( ray , SAMPLE_NUM );
            }
        }

        // get final color, if grid need to average
        average = (average / raysPerPixel * raysPerPixel);

        return average;
    }

public:

    /**
     * NOTE: I'm not using the up vector yet, just assuming it's positive y
     */

    // rayType = if we are doing ray tracing or ray marching
    // ray marching here was implemented so far only for volumetric lighthing,
    // so some other values will need to be set up before using it (ka and ks for instance)
    Camera(Point pos, Point look, Vector up, int imH, int imW, double viewH, double viewW, int rayType, int depthOrSamples, int raysPerPixel) :
        position(pos), lookAt(look), up(up), imageHeight(imH), imageWidth(imW), viewPlaneHeigth(viewH), viewPlaneWidth(viewW), rayType(rayType), raysPerPixel(raysPerPixel) {

        // each pixel
        unitsHigh = viewH/imH;
        unitsWidth = viewW/imW;

        // first pixel of our view// first pixel is at
        firstPixelx = -viewPlaneWidth*0.5;
        firstPixely = viewPlaneHeigth*0.5;

        // defining the viewing coordinates
        // oposite direction to help calculations
        w = Vector(lookAt,position,true);
        normalize(w);
        u = cross(up,w);
        normalize(u);
        v = cross(w,u);

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
    }

    std::vector<Color> render (World world) {
        // Size of canvas
        int pixelNum = imageWidth * imageHeight;

        // Result color of a ray
        std::vector<Color> colorMap(pixelNum);

        int cores = std::thread::hardware_concurrency();
        volatile std::atomic<int> count(0);
        std::vector<std::future<void> > futureVector;

        while (cores--) {
            futureVector.push_back(
                std::async([=, &colorMap, &world, &count]()
                {
                    while (true) {
                        int index = count++;
                        if (index >= pixelNum)
                            break;
                        int i = index / imageWidth;
                        int j = index % imageWidth;
                        colorMap[index] = getColorInPixel(world,i,j);
                    }
                }));
        }
/*
        // this loop is going like
        // consider origin at top left
        // fixate column
        //    go through the rows in the column
        // then go to next column

        for(int i = 0; i < imageWidth; ++i) {
            for(int j = 0; j < imageHeight; ++j) {
                colorMap.push_back( getColorInPixel(world,i,j) );
            }
        }
*/
        // will return a vector with imageWidth * imageHeight values, use it to paint the canvas
        return colorMap;
    }
};

#endif
