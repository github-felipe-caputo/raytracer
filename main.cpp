#include <cmath>
#include <iostream>

#include <SFML/Graphics.hpp>
#include "canvas.h"

using namespace std;

#define PI 3.14159265

// pixels
int imageHeight = 512;
int imageWidth = 512;

// keeping the aspect ratio of the window pixels
float viewPlaneHeigth = 0.5;
float viewPlaneWidth = 0.5;

// each pixel
float unitsHigh = viewPlaneHeigth/imageHeight;
float unitsWidth = viewPlaneWidth/imageWidth;

float focalLength = -0.5f;



void spawnRays ( float x0, float y0, float z0, int x, int y, Canvas &c );
void normalize(float &x, float &y, float &z);
bool intersectWithSphere(float x0, float y0, float z0, float dx, float dy, float dz);
bool intersectWithSphere2(float x0, float y0, float z0, float dx, float dy, float dz);
bool intersectWithFloor(float x0, float y0, float z0, float dx, float dy, float dz);

int main(void) {
    // Our canvas
    Canvas canvas( imageWidth, imageHeight );

    // maybe imageWidth-1 and imageHeight-1
    for(int i = 0; i < imageWidth; ++i)
        for(int j = 0; j < imageHeight; ++j)
            spawnRays(0,0,0,i,j,canvas);

    sf::RenderWindow window(sf::VideoMode(imageWidth, imageHeight), "Ray Tracer");

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

// x0,y0,z0 = ray origin
// x, y = actual pixel in the screen we'll set
// focal length = 0.5
void spawnRays ( float x0, float y0, float z0, int x, int y, Canvas &c) {
    // ""first pixel"" is at
    float firstPlanex = -viewPlaneWidth/2;// + unitsWidth/2);
    float firstPlaney = viewPlaneHeigth/2;// + unitsHigh/2);

    // create a ray
    // have origin, need direction
    float dx,dy,dz;
    dx = firstPlanex + (unitsWidth/2) + x * unitsWidth;
    dy = firstPlaney - (unitsHigh/2) - y * unitsHigh;
    dz = focalLength;

    //cout << x  << " " <<  y  << " - " << dx << " " << dy  << " " << dz << endl;

    // normalize
    normalize(dx,dy,dz);

    // now lets see if this ray intersects with a sphere
    // pls work
    if(intersectWithFloor(x0,y0,z0,dx,dy,dz))
        c.setPixel(x,y,1,0,0);

    if(intersectWithSphere2(x0,y0,z0,dx,dy,dz))
        c.setPixel(x,y,0,0,1);

    if(intersectWithSphere(x0,y0,z0,dx,dy,dz))
        c.setPixel(x,y,0,1,0);

}

void normalize(float &x, float &y, float &z) {
    float len = sqrt(x*x+y*y+z*z);

    if (len != 0) {
        x = x / len;
        y = y / len;
        z = z / len;
    } 
}

bool intersectWithSphere(float x0, float y0, float z0, float dx, float dy, float dz){
    float xc = 0.0f;
    float yc = 0.1f;
    float zc = -2.0f;
    float r = 0.4f;

    //float A = 1;
    float B = 2.0f * (dx * (x0 - xc) + dy * (y0 - yc) + dz * (z0 - zc));
    float C = (x0 - xc)*(x0 - xc) + (y0 - yc)*(y0 - yc) + (z0 - zc)*(z0 - zc) - r*r;

    // was there a intersection?
    /*
    if (B*B - 4.0f*C >= 0) {
        cout << "true" << endl;
    } else {
        cout << "false" << endl;
    }*/
    

    return (B*B - 4.0f*C >= 0);
}

bool intersectWithSphere2(float x0, float y0, float z0, float dx, float dy, float dz){
    float xc = -0.5f;
    float yc = -0.2f;
    float zc = -2.5f;
    float r = 0.3f;

    //float A = 1;
    float B = 2.0f * (dx * (x0 - xc) + dy * (y0 - yc) + dz * (z0 - zc));
    float C = (x0 - xc)*(x0 - xc) + (y0 - yc)*(y0 - yc) + (z0 - zc)*(z0 - zc) - r*r;

    // was there a intersection?
    /*
    if (B*B - 4.0f*C >= 0) {
        cout << "true" << endl;
    } else {
        cout << "false" << endl;
    }*/
    

    return (B*B - 4.0f*C >= 0);
}

bool intersectWithFloor(float x0, float y0, float z0, float dx, float dy, float dz){
    // vertices of the floor / polygon
    float v1[] = {0.8,0.1,2.6};
    float v2[] = {0.8,0.1,-7.0};
    float v3[] = {-2.3,0.1,2.6};
    float v4[] = {-2.3,0.1,-7.0};

    float vertices[][3] = {{-0.3,0.1,2.6},{-0.3,0.1,-1.5},{0.1,0.1,-1.5},{0.1,0.1,2.6}};

    // for the plane where the floor is
    float nx = 0.0;
    float ny = 1.0;
    float nz = 0.0;
    float f = 0.1;

    // ray-plane intersection
    float w = -(nx*x0 + ny*y0 + nz*z0 + f) / (nx*dx + ny*dy + nz*dz);
    float wx, wy, wz;

    // there was a intersection
    if (w > 0) {
        // actual intersection point
        wx = x0 + dx * w;
        wz = z0 + dz * w;

        int i, j;
        bool result = false;
        for (i = 0, j = 3; i < 4; j = i++) {
            if ( ((vertices[i][2]>wz) != (vertices[j][2]>wz)) && 
                (wx < (vertices[j][0]-vertices[i][0]) * (wz-vertices[i][2]) / (vertices[j][2]-vertices[i][2]) + vertices[i][0]) )
                result = !result;
        }
        return result;
    }
    return false;
}