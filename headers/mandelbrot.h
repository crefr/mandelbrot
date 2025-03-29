#ifndef MANDELBROT_INCLUDED
#define MANDELBROT_INCLUDED

#include <stdint.h>

void calcMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                    const double left_x, const double right_x, const double bottom_y);

void calcCenteredMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                            const double center_x, const double center_y, const double scale);

void numsToColor(uint32_t * nums, uint32_t * colors, size_t len);


#endif
