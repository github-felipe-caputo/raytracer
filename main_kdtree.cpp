#include <cmath>
#include <ctime>
#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>

#include "canvas.h"
#include "mathHelper.h"
#include "object.h"
#include "world.h"
#include "camera.h"
#include "lightSource.h"
#include "illuminationModel.h"
#include "texture.h"
#include "toneReproduction.h"
#include "readPly.h"
#include "kdtree.h"

// pixels
int imageHeight = 512;
int imageWidth = 512;

// keeping the aspect ratio of the window pixels
double viewPlaneHeigth = 0.5;
double viewPlaneWidth = 0.5;

int main ( void ) {
    // SFML canvas and window
    Canvas canvas( imageWidth, imageHeight );
    sf::RenderWindow window(sf::VideoMode(imageWidth, imageHeight), "Ray Tracer");

    // Get the triangles from the bunny fily
    std::vector<Triangle> bunny = readPlyFile("plyFiles/bun_zipper", Color(1,0.2,0.2));

    // create a light source
    PointLight light( Point(0.0, 5.0, 3.0), Color(1,1,1) );

    // create world, add objects in it
    World world;

    // Add the triangles from the bunny in the world
    for (unsigned int i = 0; i < bunny.size() ; ++i) {
        Triangle *t = new Triangle( bunny[i].getPoints(), Color(1,0.2,0.2) );
        (*t).setUpPhong( Color(1,1,1), 0.6, 0.35, 0.2, 20.0 );
        world.addObject(t);
    }

    // add light and set up phong
    world.addLight(&light);
    world.setUpPhongIllumination( Color(0.25,0.61,1.00) );

    // Create Tree
    clock_t time_a = clock();
    world.createKdTree(-10,10,-10,10,-10,10);
    clock_t time_b = clock();

    std::cout << "Time to build tree (in milliseconds): " << (unsigned int)(time_b - time_a) << std::endl;


    // create camera
    Point pos(0,0.1,0.4);
    Vector lookAt(0,-0.5,0); // not being used yet
    Vector up(0,1,0); // not being used yet
    Camera cam(pos, lookAt, up, imageHeight, imageWidth, viewPlaneHeigth, viewPlaneWidth, 
        RAY_TRACER, 1, RAY_CENTER);

    // render our world, get the color map we will put on canvas
    clock_t time_c = clock();
    std::vector<Color> colorMap = cam.render(world);
    clock_t time_d = clock();
    std::cout << "Time to render (in milliseconds): " << (unsigned int)(time_d - time_c) << std::endl;

    // Tonemap
    std::vector<Color> toneReprodColorMap = compressionPerceptual(colorMap , 100);

    // set pixel values on the canvas
    for(int i = 0; i < imageWidth; ++i) {
        for(int j = 0; j < imageHeight; ++j) {
            Color c = toneReprodColorMap[i * imageWidth + j];
            canvas.setPixel( i, j, c.r, c.g, c.b );
        }
    }
    
    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...
        canvas.draw(window);

        // end the current frame
        window.display();
    }

    return 0;
}