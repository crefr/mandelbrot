#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <SFML/Graphics.hpp>

#include "mandelbrot.h"

const double   POS_CHANGE_COEF = 0.1;
const double SCALE_CHANGE_COEF = 1.1;
const uint32_t ITER_NUM_DELTA = 128;

const char * POS_FILE_NAME = "position.txt";

#define PRINT_TIME(function)                                                                                    \
do {                                                                                                            \
    struct timespec calc_start = {};                                                                            \
    struct timespec calc_end = {};                                                                              \
    clock_gettime(CLOCK_MONOTONIC, &calc_start);                                                                \
    function;                                                                                                   \
    clock_gettime(CLOCK_MONOTONIC, &calc_end);                                                                  \
    double calc_time = 1000 * (calc_end.tv_sec - calc_start.tv_sec) + (calc_end.tv_nsec - calc_start.tv_nsec) / 1e6;   \
    printf("%lf ms\n", calc_time);                                                                              \
} while(0)

static void handlePressedKey(sf::Keyboard::Key pressed_key_code, mandelbrot_context_t * md);

static void savePositionToFile(const char * file_name, mandelbrot_context_t * md);

static void readPositionFromFile(const char * file_name, mandelbrot_context_t * md);

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
                handlePressedKey(event.key.code, &md);
            }

            if (event.type == sf::Event::MouseWheelScrolled){
                md.scale = (event.mouseWheelScroll.delta > 0) ?
                    md.scale / SCALE_CHANGE_COEF :
                    md.scale * SCALE_CHANGE_COEF;
            }
        }

        /***************************/
        printf("one frame calc time = ");
        // PRINT_TIME(calcMandelbrotMultiThread(&md, 8));
        PRINT_TIME(calcMandelbrot(&md));
        // PRINT_TIME(calcMandelbrotGCCoptimized(&md));
        // PRINT_TIME(calcMandelbrotNoOptimization(&md));

        printf("one frame coloring time = ");
        PRINT_TIME(numsToColor(&md));
        /***************************/

        mandelbrot_texture.update((uint8_t *)md.color_pixels);

        window.clear();

        window.draw(mandelbrot_sprite);

        window.display();
    }
    mandelbrotDtor(&md);
}

static void handlePressedKey(sf::Keyboard::Key pressed_key_code, mandelbrot_context_t * md)
{
    assert(md);

    float step = md->sc_width * md->scale * POS_CHANGE_COEF;

    switch(pressed_key_code){
        case sf::Keyboard::Escape:
            md->center_x = DEFAULT_CENTER_X;
            md->center_y = DEFAULT_CENTER_Y;
            md->iter_num = DEFAULT_ITER_NUM;
            md->scale = DEFAULT_PLOT_WIDTH / md->sc_width;
            break;

        case sf::Keyboard::F6:
            savePositionToFile(POS_FILE_NAME, md);
            break;

        case sf::Keyboard::F9:
            readPositionFromFile(POS_FILE_NAME, md);
            break;

        case sf::Keyboard::W: case sf::Keyboard::Up:
            md->center_y -= step;
            break;

        case sf::Keyboard::S: case sf::Keyboard::Down:
            md->center_y += step;
            break;

        case sf::Keyboard::A: case sf::Keyboard::Left:
            md->center_x -= step;
            break;

        case sf::Keyboard::D: case sf::Keyboard::Right:
            md->center_x += step;
            break;

        case sf::Keyboard::Z:
            if (md->iter_num > ITER_NUM_DELTA)
                md->iter_num -= ITER_NUM_DELTA;
            break;

        case sf::Keyboard::X:
            md->iter_num += ITER_NUM_DELTA;
            break;

        default:
            break;
    }
}

static void savePositionToFile(const char * file_name, mandelbrot_context_t * md)
{
    assert(file_name);
    assert(md);

    FILE * pos_file = fopen(file_name, "w");

    if (! pos_file){
        fprintf(stderr, "ERROR: Could not open file '%s' for position saving\n", file_name);
        return;
    }

    fprintf(pos_file,
        "center_x   = %lf\n"
        "center_y   = %lf\n"
        "iter_num   = %u\n"
        "plot_width = %lf\n\n",
        md->center_x, md->center_y, md->iter_num, md->scale * md->sc_width);

    fclose(pos_file);
}

static void readPositionFromFile(const char * file_name, mandelbrot_context_t * md)
{
    assert(file_name);
    assert(md);

    FILE * pos_file = fopen(file_name, "r");

    if (! pos_file){
        fprintf(stderr, "ERROR: Could not open file '%s' for position reading\n", file_name);
        return;
    }

    double center_x = 0;
    double center_y = 0;
    uint32_t iter_num = 0;
    double plot_width = 0;

    int scan_result  = fscanf(pos_file,
        "center_x   = %lf\n"
        "center_y   = %lf\n"
        "iter_num   = %u\n"
        "plot_width = %lf\n\n",
        &center_x, &center_y, &iter_num, &plot_width);

    if (scan_result != 4){
        fprintf(stderr, "ERROR: Incorrect file format for reading ('%s')\n", file_name);
        fclose(pos_file);
        return;
    }
    md->center_x = center_x;
    md->center_y = center_y;
    md->iter_num = iter_num;

    md->scale    = plot_width / md->sc_width;

    fclose(pos_file);
}
