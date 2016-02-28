#include <cmath>
#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>

#include "canvas.h"
#include "mathHelper.h"
#include "object.h"
#include "world.h"
#include "camera.h"

#define PI 3.14159265

// pixels
int imageHeight = 512;
int imageWidth = 512;

// keeping the aspect ratio of the window pixels
float viewPlaneHeigth = 0.5;
float viewPlaneWidth = 0.5;


int main(void) {
    // SFML canvas and window
    Canvas canvas( imageWidth, imageHeight );
    sf::RenderWindow window(sf::VideoMode(imageWidth, imageHeight), "Ray Tracer");

    // first create our objects
    Sphere greenSphere( Point(0.0f,0.1f,-2.0f), 0.4f, Color(0,1,0) );
    Sphere blueSphere( Point(-0.5f,-0.2f,-2.5f), 0.3f, Color(0,0,1) );

    std::vector<Point> vertices;
    vertices.push_back( Point(-1.5f,-0.6f, 0.0) );
    vertices.push_back( Point(-1.5f,-0.6f,-6.0) );
    vertices.push_back( Point( 0.5f,-0.6f,-6.0) );
    vertices.push_back( Point( 0.5f,-0.6f, 0.0) );
    Polygon redFloor( vertices, Vector(0,1,0), Color(1,0,0) );

    // create world, add objects in it
    World world;
    world.addObject(&greenSphere);
    world.addObject(&blueSphere);
    world.addObject(&redFloor);

    // create camera
    Point pos(0,0,0);
    Vector lookAt(0,-0.5f,0); // not being used yet
    Vector up(0,1,0); // not being used yet
    Camera cam(pos, lookAt, up, imageHeight, imageWidth, viewPlaneHeigth, viewPlaneWidth);

    // render our world, get the color map we will put on canvas
    std::vector<Color> colorMap = cam.render(world);

    // set pixel values on the canvas
    for(int i = 0; i < imageWidth; ++i) {
        for(int j = 0; j < imageHeight; ++j) {
            Color c = colorMap[i * imageWidth + j];
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