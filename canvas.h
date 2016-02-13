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
    Canvas ( int w, int h );

    void clear();

    void draw ( sf::RenderWindow &window );

    void setPixel ( int x, int y, float r, float g, float b );
};

#endif