#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "mandelbrot.h"

const uint32_t MAX_N = 256;
const float MAX_R2   = 100.f;

// p_n = p_{n-1}^2 + p0
// x_new + iy_new = x^2 + 2xy*i - y^2 + x0 + y0*i
// x_new = x^2 - y^2 + x0
// y_new = 2xy + y0

void calcMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                    const float left_x, const float right_x, const float bottom_y)
{
    const float top_y = ((right_x - left_x) * sc_height) / sc_width + bottom_y;

    const float dx = (right_x - left_x) / sc_width;
    const float dy = (top_y - bottom_y) / sc_height;

    for (uint32_t iy = 0; iy < sc_height; iy++){
        float y0 = bottom_y + iy * dy;

        for (uint32_t ix = 0; ix < sc_width; ix++){
            float x0 = left_x + ix * dx;

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

void calcCenteredMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                            const float center_x, const float center_y, const float scale)
{
    float left_x  = center_x - sc_width * scale / 2;
    float right_x = sc_width * scale / 2 + center_x;

    float bottom_y = center_y - sc_height * scale / 2;

    calcMandelbrot(pixels, sc_width, sc_height, left_x, right_x, bottom_y);
}

static uint32_t numToColor(const uint32_t num)
{
    if (num == 256)
        return 0;

    uint8_t red   = 256 - num;
    uint8_t green = (uint8_t)(128 + 127.* sinf32(10000./(num + 200)));
    uint8_t blue  = (uint8_t)(40. * logf(num));

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
