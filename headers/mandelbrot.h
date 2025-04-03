#ifndef MANDELBROT_INCLUDED
#define MANDELBROT_INCLUDED

#include <stdint.h>

/* DEFAULT VALUES */
const float    DEFAULT_PLOT_WIDTH = 2.0;
const float    DEFAULT_CENTER_X   = -0.5;
const float    DEFAULT_CENTER_Y   = 0.0;
const uint32_t DEFAULT_ITER_NUM = 256;

// #define BURNING_SHIP

#define GCC_OPT_PACK_SIZE 32


#define AVX_ON

// number of intrinsic commands in one pack for better conveyorization
#define INTRIN_PACK_SIZE 3

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

/// @brief mandelbrot_context_t constructor (fills fields with default values)
mandelbrot_context_t mandelbrotCtor(const uint32_t width, const uint32_t height);

/// @brief mandelbrot_context_t destructor
void mandelbrotDtor(mandelbrot_context_t * md);

/// @brief fills context.color_pixels with color codes
void numsToColor(const mandelbrot_context_t * md);


/*************** CALCULATING MANDELBROT SET FUNCTIONS ************** */

/// @brief intrinsics as main optimization
void calcMandelbrot(mandelbrot_context_t * md);

/// @brief intrinsics and better conveyorization
void calcMandelbrotConveyor(mandelbrot_context_t * md);

/// @brief bad realisation of multithreading
void calcMandelbrotMultiThread(mandelbrot_context_t * md, size_t threads_num);

/// @brief divided into small loops for compiler avtovectorization
void calcMandelbrotGCCoptimized(mandelbrot_context_t * md);

/// @brief basic version without any optimizations
void calcMandelbrotNoOptimization(mandelbrot_context_t * md);

/******************************************************************* */


#endif
