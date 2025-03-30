#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SFML/Graphics.hpp>

#include "mandelbrot.h"

const double   POS_CHANGE_COEF = 0.1;
const double SCALE_CHANGE_COEF = 1.1;
const uint32_t ITER_NUM_DELTA = 128;

void runWindow(const uint32_t width, const uint32_t height)
{
    sf::RenderWindow window(sf::VideoMode(width, height), "Mandelbrot");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    sf::Texture mandelbrot_texture;
    mandelbrot_texture.create(width, height);

    sf::Sprite mandelbrot_sprite;
    mandelbrot_sprite.setTexture(mandelbrot_texture);

    mandelbrot_context_t md = mandelbrotCtor(width, height);

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed){
                window.close();
            }

            if (event.type == sf::Event::KeyPressed){
                float step = width * md.scale * POS_CHANGE_COEF;

                switch(event.key.code){
                    case sf::Keyboard::W: case sf::Keyboard::Up:
                        md.center_y -= step;
                        break;

                    case sf::Keyboard::S: case sf::Keyboard::Down:
                        md.center_y += step;
                        break;

                    case sf::Keyboard::A: case sf::Keyboard::Left:
                        md.center_x -= step;
                        break;

                    case sf::Keyboard::D: case sf::Keyboard::Right:
                        md.center_x += step;
                        break;

                    case sf::Keyboard::Z:
                        if (md.iter_num > ITER_NUM_DELTA)
                            md.iter_num -= ITER_NUM_DELTA;
                        break;

                    case sf::Keyboard::X:
                        md.iter_num += ITER_NUM_DELTA;
                        break;

                    default:
                        break;
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled){
                md.scale = (event.mouseWheelScroll.delta > 0) ?
                    md.scale / SCALE_CHANGE_COEF :
                    md.scale * SCALE_CHANGE_COEF;
            }

        }

        clock_t calc_start = clock();
        calcMandelbrot(&md);
        clock_t calc_end   = clock();

        printf("one frame calc time = %lf ms\n", (double)(calc_end - calc_start) / CLOCKS_PER_SEC * 1000);

        calc_start = clock();
        numsToColor(&md);
        calc_end   = clock();

        printf("one frame coloring time = %lf ms\n", (double)(calc_end - calc_start) / CLOCKS_PER_SEC * 1000);

        mandelbrot_texture.update((uint8_t *)md.color_pixels);

        window.clear();

        window.draw(mandelbrot_sprite);

        window.display();
    }
    mandelbrotDtor(&md);
}
