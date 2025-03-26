#include <stdio.h>
#include <stdlib.h>

#include <SFML/Graphics.hpp>

#include "mandelbrot.h"

void runWindow(const uint32_t width, const uint32_t height)
{
    sf::RenderWindow window(sf::VideoMode(width, height), "Mandelbrot");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    sf::Texture mandelbrot_texture;
    mandelbrot_texture.create(width, height);

    sf::Sprite mandelbrot_sprite;
    mandelbrot_sprite.setTexture(mandelbrot_texture);

    uint32_t * num_pixels   = (uint32_t *)calloc(width * height, sizeof(*num_pixels));
    uint32_t * color_pixels = (uint32_t *)calloc(width * height, sizeof(*color_pixels));


    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed){
                window.close();
            }
        }

        calcMandelbrot(num_pixels, width, height, -2, 2, -1.5, 1.5);
        numsToColor(num_pixels, color_pixels, width * height);

        mandelbrot_texture.update((uint8_t *)color_pixels);

        window.clear();

        window.draw(mandelbrot_sprite);

        window.display();
    }
    free(color_pixels);
    free(num_pixels);
}
