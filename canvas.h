#ifndef _CANVAS_H
#define _CANVAS_H

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

/*
 * The Canvas class.
 */
class Canvas {
    // Canvas size
    int width;
    int height;

    // Image
    sf::Image myImage;

public:

    /* 
     * Canvas
     *
     * INPUT: 
     *         
     *
     * DESCRIPTION:
     *
     */
    Canvas( int w , int h ) : width(w), height(h) {
        myImage.create( w, h, sf::Color(0, 0, 0) );
    }

    void clear() {
        // set all pixels to black
        for (int i = 0; i < width; ++i)
            for (int j = 0; j < height; ++j)
                myImage.setPixel (i, j, sf::Color(0, 0, 0));
    }

    // Creating myImage -> Texture -> Sprite
    void draw( sf::RenderWindow &window ) {
        sf::Texture tex;
        tex.loadFromImage( myImage );

        sf::Sprite toDraw;
        toDraw.setTexture( tex );

        // Add your image as a sprite
        window.draw( toDraw );
    }

    void setPixel ( int x, int y, float r, float g, float b ) {
        // Wrap color values if they are over 1
        if (r > 1) r = 1;
        if (g > 1) g = 1;
        if (b > 1) b = 1;

        // RGB values
        unsigned int R = (unsigned int)(r * 255);
        unsigned int G = (unsigned int)(g * 255);
        unsigned int B = (unsigned int)(b * 255);

        // Set color
        myImage.setPixel (x, y, sf::Color (R, G, B));
    }
};

#endif