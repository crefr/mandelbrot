#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

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

const float MAX_R2   = 100.;

mandelbrot_context_t mandelbrotCtor(const uint32_t width, const uint32_t height)
{
    mandelbrot_context_t md = {};

    md.num_pixels   = (uint32_t *)calloc(width * height, sizeof(*(md.num_pixels)));
    md.color_pixels = (uint32_t *)calloc(width * height, sizeof(*(md.color_pixels)));

    md.scale = DEFAULT_PLOT_WIDTH / width;

    md.center_x = DEFAULT_CENTER_X;
    md.center_y = DEFAULT_CENTER_Y;

    md.sc_width  = width;
    md.sc_height = height;

    md.iter_num = DEFAULT_ITER_NUM;

    return md;
}

void mandelbrotDtor(mandelbrot_context_t * md)
{
    assert(md);

    free(md->num_pixels);
    free(md->color_pixels);
}

// p_n = p_{n-1}^2 + p0
// x_new + iy_new = x^2 + 2xy*i - y^2 + x0 + y0*i
// x_new = x^2 - y^2 + x0
// y_new = 2xy + y0

void calcMandelbrot(mandelbrot_context_t * md)
{
    assert(md);

    const uint32_t sc_width  = md->sc_width;
    const uint32_t sc_height = md->sc_height;
    const uint32_t iter_num  = md->iter_num;

    const float left_x  = md->center_x - md->sc_width * md->scale / 2;
    const float right_x = md->sc_width * md->scale / 2 + md->center_x;

    const float bottom_y = md->center_y - md->sc_height * md->scale / 2;

    const float dx = (right_x - left_x) / md->sc_width;
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

            for (uint32_t iteration = 0; iteration < iter_num; iteration++){
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
            mXXXi * store_addr = (mXXXi *)(md->num_pixels + iy * sc_width + ix);
            mm_storeu_siXXX(store_addr, n);
        }
    }
}

static void * threadCalcMandelbrot(void * md_ptr)
{
    assert(md_ptr);

    mandelbrot_context_t * md = (mandelbrot_context_t *)md_ptr;
    calcMandelbrot(md);

    return NULL;
}

void calcMandelbrotMultiThread(mandelbrot_context_t * md, size_t threads_num)
{
    assert(md);

    const size_t MAX_THREAD_NUM = 32;

    mandelbrot_context_t thread_md_context[MAX_THREAD_NUM] = {};
    pthread_t threads[MAX_THREAD_NUM] = {};

    size_t rows_per_thread = md->sc_height / threads_num;
    size_t pixels_per_thread = rows_per_thread * md->sc_width;

    double rows_div_2_scale   = rows_per_thread * md->scale / 2;
    double height_div_2_scale = md->sc_height * md->scale / 2;

    for (size_t thread_index = 0; thread_index < threads_num; thread_index++){
        memcpy(thread_md_context + thread_index, md, sizeof(mandelbrot_context_t));

        thread_md_context[thread_index].color_pixels = NULL;
        thread_md_context[thread_index].sc_height = rows_per_thread;
        thread_md_context[thread_index].center_y += rows_div_2_scale * (2*thread_index + 1) - height_div_2_scale;
        thread_md_context[thread_index].num_pixels += pixels_per_thread * thread_index;

        pthread_create(threads + thread_index, NULL, threadCalcMandelbrot, thread_md_context + thread_index);
    }

    for (size_t thread_index = 0; thread_index < threads_num; thread_index++){
        pthread_join(threads[thread_index], NULL);
    }
}

static uint32_t numToColor(const uint32_t num, const uint32_t iter_num)
{
    if (num == iter_num)
        return 0;

    uint8_t red   = 256 - num;
    uint8_t green = (uint8_t)(128 + 127.* sinf32(10000./(num + 200)));
    uint8_t blue  = (uint8_t)(40. * logf(num));

    uint8_t alpha = 255;

    // it is rgba in sfml
    uint32_t color = (alpha << 24) | (blue << 16) | (green << 8) | red;

    return color;
}

void numsToColor(const mandelbrot_context_t * md)
{
    assert(md);

    const uint32_t len = md->sc_height * md->sc_width;

    const uint32_t iter_num = md->iter_num;

    uint32_t * color_pixels = md->color_pixels;
    const uint32_t * num_pixels = md->num_pixels;

    for (uint32_t num_index = 0; num_index < len; num_index++){
        color_pixels[num_index] = numToColor(num_pixels[num_index], iter_num);
    }
}
