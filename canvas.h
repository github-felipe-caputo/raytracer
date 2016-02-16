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
        for (int i=0; i < width; ++i)
            for (int j=0; j < height; ++j)
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
        // RGB values
        unsigned char R = (unsigned char)(r * 255);
        unsigned char G = (unsigned char)(g * 255);
        unsigned char B = (unsigned char)(b * 255);

        // Set color
        myImage.setPixel (x, y, sf::Color (R, G, B));
    }
};

#endif