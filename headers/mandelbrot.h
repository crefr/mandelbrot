#ifndef MANDELBROT_INCLUDED
#define MANDELBROT_INCLUDED

#include <stdint.h>

void calcMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                    const float left_x, const float right_x, const float bottom_y);

void calcCenteredMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                            const float center_x, const float center_y, const float scale);

void numsToColor(uint32_t * nums, uint32_t * colors, size_t len);


#endif
