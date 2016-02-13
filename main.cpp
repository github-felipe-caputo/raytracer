#include <SFML/Graphics.hpp>
#include "canvas.h"

int main(void) {
    // Our canvas
    Canvas canvas( 800, 600 );

    canvas.setPixel(400,300,1,1,1);

    sf::RenderWindow window(sf::VideoMode(800, 600), "Ray Tracer");

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