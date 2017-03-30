#ifndef _CAMERA_H
#define _CAMERA_H

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

    // number of rays we will use per pixel
    int raysPerPixel;

    // This function is given the world and the pixel, it will return the color
    // of that pixel. In other words i ranges from [0,imageWidth] and
    // j ranges from [0,imageHeight]
    Color getColorInPixel(World world, int i, int j) {
        // ray direction
        double dx,dy,dz;

        // Color average
        Color average;

        double startx = (firstPixelx + i * unitsWidth);
        double starty = (firstPixely - j * unitsHigh);

        for(int a = 0; a < raysPerPixel; ++a) {
            double randx = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/unitsWidth));
            double randy = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/unitsHigh));

            dx = startx + randx ;
            dy = starty - randy ;
            dz = focalLength;

            Vector dir = dx*u + dy*v - dz*w;
            normalize(dir);

            // ray
            Ray ray(position, dir);

            // Color average
            average += world.spawn( ray , MAX_DEPTH );
        }

        // get final color, if grid need to average
        average = (average / static_cast <double> (raysPerPixel));

        return average;
    }

public:

    // rayType = if we are doing ray tracing or ray marching
    // ray marching here was implemented so far only for volumetric lighthing,
    // so some other values will need to be set up before using it (ka and ks for instance)
    Camera(Point pos, Point look, Vector up, int imH, int imW, double viewH, double viewW, int depthOrSamples, int raysPerPixel) :
        position(pos), lookAt(look), up(up), imageHeight(imH), imageWidth(imW), viewPlaneHeigth(viewH), viewPlaneWidth(viewW), raysPerPixel(raysPerPixel) {

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

        MAX_DEPTH = depthOrSamples;
    }

    std::vector<Color> render (World world) {
        // Size of canvas
        int pixelNum = imageWidth * imageHeight;

        //double tenPercentIncrement = pixelNum * 0.1;
        //int tenPercentIncrementInt = 10;

        int cores = std::thread::hardware_concurrency();
        volatile std::atomic<int> count(0);
        volatile std::atomic<double> tenPercentIncrement(0.01);
        std::vector<std::future<void> > futureVector;

        #ifdef MULTI_THREADED
            std::cout << "Status: Using multi threaded ray tracer." << std::endl;

            // Result color of a ray
            std::vector<Color> colorMap(pixelNum);

            while (cores--) {
                futureVector.push_back(
                    std::async([=, &colorMap, &world, &count, &tenPercentIncrement]()
                    {
                        while (true) {
                            int index = count++;
                            if (index >= pixelNum)
                                break;
                            if (index > pixelNum * tenPercentIncrement) {
                                std::cout << "Status: Image processing: " << 100 * tenPercentIncrement << "% complete..." << std::endl;
                                tenPercentIncrement = 0.01 + tenPercentIncrement;
                            }
                            int i = index / imageWidth;
                            int j = index % imageWidth;
                            colorMap[index] = getColorInPixel(world,i,j);

                        }
                    }));
            }
        #else
            std::cout << "Status: Using single thread ray tracer." << std::endl;

            // Result color of a ray
            std::vector<Color> colorMap;

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
        #endif

        // will return a vector with imageWidth * imageHeight values, use it to paint the canvas
        return colorMap;
    }
};

#endif
