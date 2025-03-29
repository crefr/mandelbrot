#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "mandelbrot.h"

// #define BURNING_SHIP

#define PACK_SIZE 32

typedef double f_type;

const uint32_t MAX_N = 256;
const f_type MAX_R2   = 100.;

// p_n = p_{n-1}^2 + p0
// x_new + iy_new = x^2 + 2xy*i - y^2 + x0 + y0*i
// x_new = x^2 - y^2 + x0
// y_new = 2xy + y0

void calcMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                    const double left_x, const double right_x, const double bottom_y)
{
    // const f_type top_y = ((right_x - left_x) * sc_height) / sc_width + bottom_y;

    const f_type dx = (right_x - left_x) / sc_width;
    // const f_type dy = (top_y - bottom_y) / sc_height;
    const f_type dy = dx;

    for (uint32_t iy = 0; iy < sc_height; iy++){
        f_type y0[PACK_SIZE] = {};
        for (size_t i = 0; i < PACK_SIZE; i++) y0[i] = bottom_y + iy * dy;

        for (uint32_t ix = 0; ix < sc_width; ix += PACK_SIZE){
            f_type x0[PACK_SIZE] = {};
            for (size_t i = 0; i < PACK_SIZE; i++) x0[i] = left_x + (ix + i)*dx;

            f_type x[PACK_SIZE] = {};
            for (size_t i = 0; i < PACK_SIZE; i++) x[i] = x0[i];

            f_type y[PACK_SIZE] = {};
            for (size_t i = 0; i < PACK_SIZE; i++) y[i] = y0[i];

            int n[PACK_SIZE] = {0};

            for (uint32_t iteration = 0; iteration < MAX_N; iteration++){
                f_type x2[PACK_SIZE] = {};
                for (size_t i = 0; i < PACK_SIZE; i++) x2[i] = x[i] * x[i];

                f_type y2[PACK_SIZE] = {};
                for (size_t i = 0; i < PACK_SIZE; i++) y2[i] = y[i] * y[i];

                f_type _2xy[PACK_SIZE] = {};
                for (size_t i = 0; i < PACK_SIZE; i++) _2xy[i] = 2 * x[i] * y[i];

                int cmp_res[PACK_SIZE] = {};
                for (size_t i = 0; i < PACK_SIZE; i++) cmp_res[i] = (x2[i] + y2[i] < MAX_R2) ? 1 : 0;

                for (size_t i = 0; i < PACK_SIZE; i++) n[i] += cmp_res[i];

                uint64_t mask = 0;
                for (size_t i = 0; i < PACK_SIZE; i++){
                    mask <<= 1;
                    mask += cmp_res[i];
                }

                if (!mask)
                    break;

                #ifdef BURNING_SHIP
                    for (size_t i = 0; i < PACK_SIZE; i++) _2xy[i] = fabs(_2xy[i]);
                #endif

                for (size_t i = 0; i < PACK_SIZE; i++) x[i] = x2[i] - y2[i] + x0[i];
                for (size_t i = 0; i < PACK_SIZE; i++) y[i] = _2xy[i] + y0[i];
            }

            for (size_t i = 0; i < PACK_SIZE; i++) pixels[iy * sc_width + ix + i] = n[i];
        }
    }
}

void calcCenteredMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                            const double center_x, const double center_y, const double scale)
{
    f_type left_x  = center_x - sc_width * scale / 2;
    f_type right_x = sc_width * scale / 2 + center_x;

    f_type bottom_y = center_y - sc_height * scale / 2;

    calcMandelbrot(pixels, sc_width, sc_height, left_x, right_x, bottom_y);
}

static uint32_t numToColor(const uint32_t num)
{
    if (num == MAX_N)
        return 0;

    uint8_t red   = 256 - 256 * num / MAX_N;
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
