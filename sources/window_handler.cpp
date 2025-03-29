#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SFML/Graphics.hpp>

#include "mandelbrot.h"

const double   POS_CHANGE_COEF = 0.1;
const double SCALE_CHANGE_COEF = 1.1;

void runWindow(const uint32_t width, const uint32_t height)
{
    double scale = 2./(width);
    double center_x = -0.5,
           center_y = 0.;

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

            if (event.type == sf::Event::KeyPressed){
                float step = width * scale * POS_CHANGE_COEF;

                switch(event.key.code){
                    case sf::Keyboard::W: case sf::Keyboard::Up:
                        center_y -= step;
                        break;

                    case sf::Keyboard::S: case sf::Keyboard::Down:
                        center_y += step;
                        break;

                    case sf::Keyboard::A: case sf::Keyboard::Left:
                        center_x -= step;
                        break;

                    case sf::Keyboard::D: case sf::Keyboard::Right:
                        center_x += step;
                        break;

                    default:
                        break;
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled){
                scale = (event.mouseWheelScroll.delta > 0) ?
                    scale / SCALE_CHANGE_COEF :
                    scale * SCALE_CHANGE_COEF;
            }

        }

        clock_t calc_start = clock();
        calcCenteredMandelbrot(num_pixels, width, height, center_x, center_y, scale);
        clock_t calc_end   = clock();

        printf("one frame calc time = %lf ms\n", (double)(calc_end - calc_start) / CLOCKS_PER_SEC * 1000);

        numsToColor(num_pixels, color_pixels, width * height);

        mandelbrot_texture.update((uint8_t *)color_pixels);

        window.clear();

        window.draw(mandelbrot_sprite);

        window.display();
    }
    free(color_pixels);
    free(num_pixels);
}
