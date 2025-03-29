#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <xmmintrin.h>
#include <immintrin.h>

#include "mandelbrot.h"

// #define BURNING_SHIP

#define AVX_ON

#ifdef AVX_ON
    typedef __m256 mXXX;
    typedef __m256i mXXXi;

    #define PACK_SIZE 256

    #define mm_set_ps           _mm256_set_ps
    #define mm_set1_ps          _mm256_set1_ps

    #define mm_castsiXXX_ps     _mm256_castsi256_ps
    #define mm_castps_siXXX     _mm256_castps_si256

    #define mm_add_ps           _mm256_add_ps
    #define mm_mul_ps           _mm256_mul_ps
    #define mm_sub_ps           _mm256_sub_ps
    #define mm_and_ps           _mm256_and_ps

    #define mm_cmple_ps(a, b)   _mm256_cmp_ps((a), (b), _CMP_LE_OS)

    #define mm_movemask_ps      _mm256_movemask_ps
    #define mm_movemask_ps      _mm256_movemask_ps

    #define mm_set1_epi32       _mm256_set1_epi32
    #define mm_add_epi32        _mm256_add_epi32
    #define mm_storeu_siXXX     _mm256_storeu_si256
    #define mm_and_siXXX        _mm256_and_si256
#else
    typedef __m128  mXXX;
    typedef __m128i mXXXi;

    #define PACK_SIZE 128

    #define mm_set_ps           _mm_set_ps
    #define mm_set1_ps          _mm_set1_ps

    #define mm_castsiXXX_ps     _mm_castsi128_ps
    #define mm_castps_siXXX     _mm_castps_si128

    #define mm_add_ps           _mm_add_ps
    #define mm_mul_ps           _mm_mul_ps
    #define mm_sub_ps           _mm_sub_ps
    #define mm_and_ps           _mm_and_ps

    #define mm_cmple_ps         _mm_cmple_ps
    #define mm_movemask_ps      _mm_movemask_ps
    #define mm_movemask_ps      _mm_movemask_ps

    #define mm_set1_epi32       _mm_set1_epi32
    #define mm_add_epi32        _mm_add_epi32
    #define mm_storeu_siXXX     _mm_storeu_si128
    #define mm_and_siXXX        _mm_and_si128
#endif

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
    #ifdef AVX_ON
    const __m256 delta = _mm256_set_ps(dx*7, dx*6, dx*5, dx*4, dx*3, dx*2, dx, 0);
    #else
    const __m128 delta = _mm_set_ps(dx*3, dx*2, dx, 0);
    #endif

    const mXXX max_r2_packed = mm_set1_ps(MAX_R2);

    const mXXXi mask_for_n = mm_set1_epi32(1);

    #ifdef BURNING_SHIP
    const mXXX abs_mask = mm_castsiXXX_ps(mm_set1_epi32(~(1 << 31)));
    #endif

    for (uint32_t iy = 0; iy < sc_height; iy++){
        mXXX y0 = mm_set1_ps(bottom_y + iy * dy);

        for (uint32_t ix = 0; ix < sc_width; ix += (PACK_SIZE / 8 / sizeof(float))){
            mXXX x0 = mm_set1_ps(left_x + ix * dx);
            x0 = mm_add_ps(x0, delta);

            mXXX x = x0;
            mXXX y = y0;

            mXXXi n = mm_set1_epi32(0);

            for (uint32_t iteration = 0; iteration < MAX_N; iteration++){
                mXXX x2 = mm_mul_ps(x, x);
                mXXX y2 = mm_mul_ps(y, y);

                mXXX _2xy     = mm_mul_ps(x, y);
                mXXX packed_2 = mm_set1_ps(2.);
                _2xy = mm_mul_ps(_2xy, packed_2);

                mXXX r2 = mm_add_ps(x2, y2);

                mXXX cmp_res = mm_cmple_ps(r2, max_r2_packed);
                int mask = mm_movemask_ps(cmp_res);

                if (!mask)
                    break;

                mXXXi delta_n = mm_castps_siXXX(cmp_res);
                delta_n = mm_and_siXXX(delta_n, mask_for_n);

                n = mm_add_epi32(n, delta_n);

                mXXX sub_x2_y2 = mm_sub_ps(x2, y2);
                x = mm_add_ps(sub_x2_y2, x0);

                #ifdef BURNING_SHIP
                    _2xy = mm_and_ps(_2xy, abs_mask);
                #endif

                y = mm_add_ps(_2xy, y0);
            }
            mXXXi * store_addr = (mXXXi *)(pixels + iy * sc_width + ix);
            mm_storeu_siXXX(store_addr, n);
        }
    }
}

void calcCenteredMandelbrot(uint32_t * pixels, const uint32_t sc_width, const uint32_t sc_height,
                            const double center_x, const double center_y, const double scale)
{
    float left_x  = center_x - sc_width * scale / 2;
    float right_x = sc_width * scale / 2 + center_x;

    float bottom_y = center_y - sc_height * scale / 2;

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
