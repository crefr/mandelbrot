#ifndef MANDELBROT_INCLUDED
#define MANDELBROT_INCLUDED

#include <stdint.h>

const double DEFAULT_PLOT_WIDTH = 2.0;
const double DEFAULT_CENTER_X   = -0.5;
const double DEFAULT_CENTER_Y   = 0.0;
const uint32_t DEFAULT_ITER_NUM = 256;

typedef struct {
    uint32_t * num_pixels;
    uint32_t * color_pixels;
    double scale;
    double center_x;
    double center_y;
    uint32_t sc_width;
    uint32_t sc_height;
    uint32_t iter_num;
} mandelbrot_context_t;

mandelbrot_context_t mandelbrotCtor(const uint32_t width, const uint32_t height);

void mandelbrotDtor(mandelbrot_context_t * md);

void calcMandelbrot(mandelbrot_context_t * md);

void numsToColor(const mandelbrot_context_t * md);


#endif
