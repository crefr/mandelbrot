#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <xmmintrin.h>
#include <immintrin.h>

#include "mandelbrot.h"

// #define BURNING_SHIP

// typedef float f_type;

const uint32_t MAX_N = 256;
const float MAX_R2   = 100.;

// p_n = p_{n-1}^2 + p0
// x_new + iy_new = x^2 + 2xy*i - y^2 + x0 + y0*i
// x_new = x^2 - y^2 + x0
// y_new = 2xy + y0

void calcMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                    const double left_x, const double right_x, const double bottom_y)
{
    const float dx = (right_x - left_x) / sc_width;
    const float dy = dx;

    // reversed for easy storing in the memory
    const __m128 delta = _mm_set_ps(dx*3, dx*2, dx, 0);

    const __m128 max_r2_packed = _mm_set_ps1(MAX_R2);

    const __m128i mask_for_n = _mm_set1_epi32(1);

    #ifdef BURNING_SHIP
    const __m128 abs_mask = _mm_castsi128_ps(_mm_set1_epi32(~(1 << 31)));
    #endif

    for (uint32_t iy = 0; iy < sc_height; iy++){
        __m128 y0 = _mm_set_ps1(bottom_y + iy * dy);

        for (uint32_t ix = 0; ix < sc_width; ix += (128 / 8 / sizeof(float))){
            __m128 x0 = _mm_set_ps1(left_x + ix * dx);
            x0 = _mm_add_ps(x0, delta);

            __m128 x = x0;
            __m128 y = y0;

            __m128i n = _mm_set1_epi32(0);

            for (uint32_t iteration = 0; iteration < MAX_N; iteration++){
                __m128 x2 = _mm_mul_ps(x, x);
                __m128 y2 = _mm_mul_ps(y, y);

                __m128 _2xy     = _mm_mul_ps(x, y);
                __m128 packed_2 = _mm_set_ps1(2.);
                _2xy = _mm_mul_ps(_2xy, packed_2);

                __m128 r2 = _mm_add_ps(x2, y2);

                __m128 cmp_res = _mm_cmple_ps(r2, max_r2_packed);
                int mask = _mm_movemask_ps(cmp_res);

                if (!mask)
                    break;

                __m128i delta_n = _mm_castps_si128(cmp_res);
                delta_n = _mm_and_si128(delta_n, mask_for_n);

                n = _mm_add_epi32(n, delta_n);

                __m128 sub_x2_y2 = _mm_sub_ps(x2, y2);
                x = _mm_add_ps(sub_x2_y2, x0);

                #ifdef BURNING_SHIP
                    _2xy = _mm_and_ps(_2xy, abs_mask);
                #endif

                y = _mm_add_ps(_2xy, y0);
            }
            __m128i * store_addr = (__m128i *)(pixels + iy * sc_width + ix);
            _mm_storeu_si128(store_addr, n);
        }
    }
}

void calcMandelbrotAVX(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                    const double left_x, const double right_x, const double bottom_y)
{
    const float dx = (right_x - left_x) / sc_width;
    const float dy = dx;

    // reversed for easy storing in the memory
    const __m256 delta = _mm256_set_ps(dx*7, dx*6, dx*5, dx*4, dx*3, dx*2, dx, 0);

    const __m256 max_r2_packed = _mm256_set1_ps(MAX_R2);

    const __m256i mask_for_n = _mm256_set1_epi32(1);

    #ifdef BURNING_SHIP
    const __m256 abs_mask = _mm256_castsi256_ps(_mm256_set1_epi32(~(1 << 31)));
    #endif

    for (uint32_t iy = 0; iy < sc_height; iy++){
        float y0f = bottom_y + iy * dy;
        __m256 y0 = _mm256_set1_ps(y0f);

        for (uint32_t ix = 0; ix < sc_width; ix += (256 / 8 / sizeof(float))){
            float x0f = left_x + ix * dx;
            __m256 x0 = _mm256_set1_ps(x0f);
            x0 = _mm256_add_ps(x0, delta);

            __m256 x = x0;
            __m256 y = y0;

            __m256i n = _mm256_set1_epi32(0);

            for (uint32_t iteration = 0; iteration < MAX_N; iteration++){
                __m256 x2 = _mm256_mul_ps(x, x);
                __m256 y2 = _mm256_mul_ps(y, y);

                __m256 _2xy     = _mm256_mul_ps(x, y);
                __m256 packed_2 = _mm256_set1_ps(2.);
                _2xy = _mm256_mul_ps(_2xy, packed_2);

                __m256 r2 = _mm256_add_ps(x2, y2);

                __m256 cmp_res = _mm256_cmp_ps(r2, max_r2_packed, _CMP_LE_OS);
                int mask = _mm256_movemask_ps(cmp_res);

                if (!mask)
                    break;

                __m256i delta_n = _mm256_castps_si256(cmp_res);
                delta_n = _mm256_and_si256(delta_n, mask_for_n);

                n = _mm256_add_epi32(n, delta_n);

                __m256 sub_x2_y2 = _mm256_sub_ps(x2, y2);
                x = _mm256_add_ps(sub_x2_y2, x0);

                #ifdef BURNING_SHIP
                    _2xy = _mm256_and_ps(_2xy, abs_mask);
                #endif

                y = _mm256_add_ps(_2xy, y0);
            }
            __m256i * store_addr = (__m256i *)(pixels + iy * sc_width + ix);
            _mm256_storeu_si256(store_addr, n);
        }
    }
}

void calcCenteredMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                            const double center_x, const double center_y, const double scale)
{
    float left_x  = center_x - sc_width * scale / 2;
    float right_x = sc_width * scale / 2 + center_x;

    float bottom_y = center_y - sc_height * scale / 2;

    calcMandelbrotAVX(pixels, sc_width, sc_height, left_x, right_x, bottom_y);
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
