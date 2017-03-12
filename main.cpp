#include <cmath>
#include <ctime>
#include <iostream>
#include <vector>
#include <cstdlib>

#include <SFML/Graphics.hpp>

// defines for certain operations
//#define KD_TREE
#define MULTI_THREADED
#define CANVAS_DISPLAY

#include "canvas.h"
#include "mathHelper.h"
#include "transform.h"
#include "object.h"
#include "world.h"
#include "camera.h"
#include "lightSource.h"
#include "illuminationModel.h"
#include "proceduralTexture.h"
#include "toneReproduction.h"

#include "readPly.h"
#include "kdtree.h"

// pixels
int imageHeight = 1024;
int imageWidth = 1024;

// keeping the aspect ratio of the window pixels
double viewPlaneHeigth = 0.5;
double viewPlaneWidth = 0.5;

int main ( void ) {
    // set up random number seed
    srand (static_cast <unsigned> (time(0)));

    // first create our objects
    Sphere frontSphere( Point(0.0,0.1,-1.9), 0.4, Color(1,1,1) );
    //frontSphere.setUpPhong( Color(1,1,1), 0.2, 0.5, 1.0, 50.0 );
    frontSphere.setUpPhong( Color(1,1,1), 0.075, 0.075, 0.2, 20.0 );
    frontSphere.setUpReflectionTransmission(0.0, 0.8, 0.95);

    //Sphere frontSphere( Point(0.0,0.1,-1.9), 0.4, Texture("textures/earth.jpg") );
    //frontSphere.setUpPhong( Color(1,1,1), 1.0, 1.0, 0.0, 1.0 );
    //frontSphere.setUpReflectionTransmission(0.0, 0.0, 0.0);

    Sphere backSphere( Point(0,0,0), 0.3, Color(0.7,0.7,0.7) );
    backSphere.setUpPhong( Color(1,1,1), 0.15, 0.25, 1.0, 20.0 );
    //backSphere.setUpReflectionTransmission(0.75, 0.0, 1.0);

    // FLOOR
    std::vector<Point> vertices;
    vertices.push_back( Point(-1,0, 1) );
    vertices.push_back( Point(-1,0,-1) );
    vertices.push_back( Point( 1,0,-1) );
    vertices.push_back( Point( 1,0, 1) );
    Rectangle floorRectangle( vertices, Color(1.0,0.0,0.0) );
    floorRectangle.setUpPhong( Color(0.9,0.9,0.9), 0.5, 1.0, 0.0, 1.0 );

    // CEILING
    vertices.clear();
    vertices.push_back( Point( 1,0.0, 1) );
    vertices.push_back( Point( 1,0.0,-1) );
    vertices.push_back( Point(-1,0.0,-1) );
    vertices.push_back( Point(-1,0.0, 1) );
    Rectangle ceilingRectangle( vertices, Color(0.0,1.0,0.0) );
    ceilingRectangle.setUpPhong( Color(0.9,0.9,0.9), 0.5, 1.0, 0.0, 1.0 );

    // LEFT
    vertices.clear();
    vertices.push_back( Point( 0.0, 1, 1) );
    vertices.push_back( Point( 0.0, 1,-1) );
    vertices.push_back( Point( 0.0,-1,-1) );
    vertices.push_back( Point( 0.0,-1, 1) );
    Rectangle leftRectangle( vertices, Color(0.0,0.0,1.0) );
    leftRectangle.setUpPhong( Color(0.9,0.9,0.9), 0.5, 1.0, 0.0, 1.0 );

    // RIGHT
    vertices.clear();
    vertices.push_back( Point( 0.0,-1, 1) );
    vertices.push_back( Point( 0.0,-1,-1) );
    vertices.push_back( Point( 0.0, 1,-1) );
    vertices.push_back( Point( 0.0, 1, 1) );
    Rectangle rightRectangle( vertices, Color(1.0,0.0,1.0) );
    rightRectangle.setUpPhong( Color(0.9,0.9,0.9), 0.5, 1.0, 0.0, 1.0 );

    // FORWARD
    vertices.clear();
    vertices.push_back( Point(  1, 1, 0.0) );
    vertices.push_back( Point(  1,-1, 0.0) );
    vertices.push_back( Point( -1,-1, 0.0) );
    vertices.push_back( Point( -1, 1, 0.0) );
    Rectangle forwardRectangle( vertices, Color(0.0,1.0,1.0) );
    forwardRectangle.setUpPhong( Color(0.9,0.9,0.9), 0.5, 1.0, 0.0, 1.0 );

    // BACK
    vertices.clear();
    vertices.push_back( Point( -1, 1, 0.0) );
    vertices.push_back( Point( -1,-1, 0.0) );
    vertices.push_back( Point(  1,-1, 0.0) );
    vertices.push_back( Point(  1, 1, 0.0) );
    Rectangle backRectangle( vertices, Color(0.0,0.0,0.0) );
    backRectangle.setUpPhong( Color(0.9,0.9,0.9), 0.5, 1.0, 0.0, 1.0 );



    // Get the triangles from the bunny fily
    std::vector<Triangle> bunny = readPlyFile("plyFiles/bun_zipper_res4", Color(1,0.2,0.2));



    // create a light source
    PointLight light( Point(0, 0, -3), Color(1,1,1) );
    //PointLight light2( Point(-5.0f, 3.0f, 0.0f), Color(1,1,1) );

    Sphere sphereLightObj( Point(0.0, 0.0, 0.0), 0.3, Color(1,1,1) );
    sphereLightObj.setUpEmissionColor( Color(1,1,1) );
    AreaLight sphereLight( &sphereLightObj, 16 );

    // rectangle light
    std::vector<Point> v;
    //v.push_back( Point(-1.5,-0.6, 0.0) );
    //v.push_back( Point(-1.5,-0.6,-6.0) );
    //v.push_back( Point( 0.5,-0.6,-6.0) );
    //v.push_back( Point( 0.5,-0.6, 0.0) );
    v.push_back( Point( 0.5,0.0, 0.5) );
    v.push_back( Point( 0.5,0.0,-0.5) );
    v.push_back( Point(-0.5,0.0,-0.5) );
    v.push_back( Point(-0.5,0.0, 0.5) );
    Rectangle rectangleLightObj( v, Color(1,1,1) );
    rectangleLightObj.setUpEmissionColor( Color(1,1,1) );
    AreaLight rectangleLight( &rectangleLightObj, 16 );

    // transforms

    // light
    translate(&sphereLightObj, 0, 0.0, -3);
    translate(&rectangleLightObj, 0, 0.8, -3);

    // walls
    translate(&floorRectangle, 0, -1, -3);
    translate(&ceilingRectangle, 0, 1, -3);
    translate(&leftRectangle, -1, 0, -3);
    translate(&rightRectangle, 1, 0, -3);
    translate(&forwardRectangle, 0, 0, -4);
    translate(&backRectangle, 0, 0, -2);

    // objects
    translate(&backSphere, 0, 0, -3);



/*
    for (unsigned int i = 0; i < bunny.size() ; ++i) {
        scale(&bunny[i], 0.5, 0.5, 0.5);
        translate(&bunny[i], 0, -0.1, -0.3);
    }
*/
    // create world, add objects in it
    World world;
    //world.addObject(&frontSphere);
    //world.addObject(&backSphere);

    //world.addObject(&floorRectangle);
    //world.addObject(&ceilingRectangle);
    world.addObject(&leftRectangle);
    world.addObject(&rightRectangle);
    //world.addObject(&forwardRectangle);
    //world.addObject(&backRectangle);

    world.addObject(&sphereLightObj); // full white
    //world.addObject(&rectangleLightObj); // full white
/*
    // Add the triangles from the bunny in the world
    for (unsigned int i = 0; i < bunny.size() ; ++i) {
        bunny[i].setUpPhong( Color(1,1,1), 0.6, 0.35, 0.2, 20.0 );
        world.addObject(&bunny[i]);
    }
*/
    // add light and set up phong
    //world.addLight(&light);
    //world.addLight(&light2);
    world.addLight(&sphereLight);
    //world.addLight(&rectangleLight);
    //world.setUpPhongBlinnIllumination( Color(0.25,0.61,1.00) );
    world.setUpPhongIllumination( Color(0.25,0.61,1.00) );
    //world.setUpPhongIllumination( Color(0.8,0.2,0.2) ); // RED

    #ifdef KD_TREE
        std::cout << "Status: Using KD Tree." << std::endl;

        // Create Tree
        world.createKdTree(-5,5,-5,5,-5,5);
    #else
        std::cout << "Status: Using regular ray traversal." << std::endl;
    #endif

    // create camera
    Point pos(0.0,0.0,1.0);
    Vector up(0.0,1.0,0.0);
    Point lookAt(0.0,0.0,-1.0);
    Camera cam(pos, lookAt, up, imageHeight, imageWidth, viewPlaneHeigth, viewPlaneWidth, 1, 1);

    // render our world, get the color map we will put on canvas
    std::vector<Color> colorMap = cam.render(world);

    // Tone reproduction
    std::vector<Color> toneReprodColorMap = compressionPerceptual(colorMap , 100);
    //std::vector<Color> toneReprodColorMap = colorMap;

    // SFML canvas and window
    Canvas canvas( imageWidth, imageHeight );
    sf::RenderWindow window(sf::VideoMode(imageWidth, imageHeight), "Ray Tracer");

    // set pixel values on the canvas
    for(int i = 0; i < imageWidth; ++i) {
        for(int j = 0; j < imageHeight; ++j) {
            Color c = toneReprodColorMap[i * imageWidth + j];
            canvas.setPixel( i, j, c.r, c.g, c.b );
        }
    }

    #ifdef CANVAS_DISPLAY
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
    #endif

    std::cout << "Status: Done." << std::endl;

    return 0;
}
