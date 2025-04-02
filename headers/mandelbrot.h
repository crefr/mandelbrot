#ifndef MANDELBROT_INCLUDED
#define MANDELBROT_INCLUDED

#include <stdint.h>

const float DEFAULT_PLOT_WIDTH = 2.0;
const float DEFAULT_CENTER_X   = -0.5;
const float DEFAULT_CENTER_Y   = 0.0;
const uint32_t DEFAULT_ITER_NUM = 256;

typedef struct {
    uint32_t * num_pixels;
    uint32_t * color_pixels;
    float scale;
    float center_x;
    float center_y;
    uint32_t sc_width;
    uint32_t sc_height;
    uint32_t iter_num;
} mandelbrot_context_t;

mandelbrot_context_t mandelbrotCtor(const uint32_t width, const uint32_t height);

void mandelbrotDtor(mandelbrot_context_t * md);

void calcMandelbrot(mandelbrot_context_t * md);

void calcMandelbrotConveyor(mandelbrot_context_t * md);

void calcMandelbrotMultiThread(mandelbrot_context_t * md, size_t threads_num);

void calcMandelbrotGCCoptimized(mandelbrot_context_t * md);

void calcMandelbrotNoOptimization(mandelbrot_context_t * md);

void numsToColor(const mandelbrot_context_t * md);


#endif
