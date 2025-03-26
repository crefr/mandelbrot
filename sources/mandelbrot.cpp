#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "mandelbrot.h"

const uint32_t MAX_N = 256;
const float MAX_R2   = 100.f;

// p_n = p_{n-1}^2 + p0
// x_new + iy_new = x^2 + 2xy*i - y^2 + x0 + y0*i
// x_new = x^2 - y^2 + x0
// y_new = 2xy + y0

void calcMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                    const float left_x, const float right_x, const float bottom_y, const float top_y)
{
    const float dx = (right_x - left_x) / sc_width;
    const float dy = (top_y - bottom_y) / sc_height;

    float y0 = bottom_y;
    for (uint32_t iy = 0; iy < sc_height; iy++, y0 += dy){
        float x0 = left_x;

        for (uint32_t ix = 0; ix < sc_width; ix++, x0 += dx){
            float x = x0;
            float y = y0;

            uint32_t n = 0;
            for (; n < MAX_N; n++){
                float x2   = x * x;
                float y2   = y * y;
                float _2xy = 2 * x * y;

                if (x2 + y2 > MAX_R2)
                    break;

                x = x2 - y2 + x0;
                y = _2xy + y0;

            }

            pixels[iy * sc_width + ix] = n;
        }
    }
}

static uint32_t numToColor(const uint32_t num)
{
    uint8_t red   = 256 - num;
    uint8_t green = 0;
    uint8_t blue  = 0;
    uint8_t alpha = 255;

    // it is rgba in sfml
    uint32_t color = (alpha << 24) | (blue << 16) | (green << 8) | red;

    return color;
}

void numsToColor(uint32_t * nums, uint32_t * colors, size_t len)
{
    for (size_t num_index = 0; num_index < len; num_index++)
        colors[num_index] = numToColor(nums[num_index]);
}
