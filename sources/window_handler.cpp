#include <stdio.h>
#include <stdlib.h>

#include <SFML/Graphics.hpp>

void runWindow(const uint32_t width, const uint32_t height)
{
    sf::RenderWindow window(sf::VideoMode(width, height), "Mandelbrot");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed){
                window.close();
            }
        }

        window.clear();

        window.display();
    }
}
