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

#ifdef AVX_ON
    typedef __m256 mXXX;
    typedef __m256i mXXXi;

    // size of ymm register in bytes
    #define PACK_SIZE 32
    #define NUMS_IN_PACK (PACK_SIZE / sizeof(float))

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

    // size of xmm register in bytes
    #define PACK_SIZE 16
    #define NUMS_IN_PACK (PACK_SIZE / sizeof(float))

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

static void calculateColorTable(const mandelbrot_context_t * md);


mandelbrot_context_t mandelbrotCtor(const uint32_t width, const uint32_t height)
{
    mandelbrot_context_t md = {};

    md.num_pixels   = (uint32_t *)calloc(width * height, sizeof(*(md.num_pixels)));
    md.color_pixels = (uint32_t *)calloc(width * height, sizeof(*(md.color_pixels)));

    md.color_table  = (uint32_t *)calloc(COLOR_TABLE_LEN, sizeof(*(md.color_table)));
    calculateColorTable(&md);

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

        for (uint32_t ix = 0; ix < sc_width; ix += NUMS_IN_PACK){
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

#define INTRIN_CYCLE for (size_t i = 0; i < INTRIN_PACK_SIZE; i++)

void calcMandelbrotConveyor(mandelbrot_context_t * md)
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
        mXXX y0[INTRIN_PACK_SIZE] = {};
        INTRIN_CYCLE y0[i] = mm_set1_ps(bottom_y + (iy) * dy);

        for (uint32_t ix = 0; ix < sc_width; ix += NUMS_IN_PACK * INTRIN_PACK_SIZE){
            mXXX x0[INTRIN_PACK_SIZE] = {};
            INTRIN_CYCLE x0[i] = mm_set1_ps(left_x + (ix + i * NUMS_IN_PACK) * dx);

            INTRIN_CYCLE x0[i] = mm_add_ps(x0[i], delta);

            mXXX x[INTRIN_PACK_SIZE] = {};
            INTRIN_CYCLE x[i] = x0[i];

            mXXX y[INTRIN_PACK_SIZE] = {};
            INTRIN_CYCLE y[i] = y0[i];

            mXXXi n[INTRIN_PACK_SIZE] = {};
            INTRIN_CYCLE n[i] = mm_set1_epi32(0);

            for (uint32_t iteration = 0; iteration < iter_num; iteration++){
                mXXX x2[INTRIN_PACK_SIZE] = {};
                INTRIN_CYCLE x2[i] = mm_mul_ps(x[i], x[i]);

                mXXX y2[INTRIN_PACK_SIZE] = {};
                INTRIN_CYCLE y2[i] = mm_mul_ps(y[i], y[i]);

                mXXX _2xy[INTRIN_PACK_SIZE] = {};
                INTRIN_CYCLE _2xy[i] = mm_mul_ps(x[i], y[i]);
                INTRIN_CYCLE _2xy[i] = mm_add_ps(_2xy[i], _2xy[i]);

                mXXX r2[INTRIN_PACK_SIZE] = {};
                INTRIN_CYCLE r2[i] = mm_add_ps(x2[i], y2[i]);

                mXXX cmp_res[INTRIN_PACK_SIZE] = {};
                INTRIN_CYCLE cmp_res[i] = mm_cmple_ps(r2[i], max_r2_packed);

                int continue_calc = 0;
                INTRIN_CYCLE {
                    int mask = mm_movemask_ps(cmp_res[i]);
                    continue_calc |= mask;
                }
                if (! continue_calc)
                    break;

                mXXXi delta_n[INTRIN_PACK_SIZE] = {};
                INTRIN_CYCLE delta_n[i] = mm_castps_siXXX(cmp_res[i]);
                INTRIN_CYCLE delta_n[i] = mm_and_siXXX(delta_n[i], mask_for_n);

                INTRIN_CYCLE n[i] = mm_add_epi32(n[i], delta_n[i]);

                mXXX sub_x2_y2[INTRIN_PACK_SIZE] = {};
                INTRIN_CYCLE sub_x2_y2[i] = mm_sub_ps(x2[i], y2[i]);

                INTRIN_CYCLE x[i] = mm_add_ps(sub_x2_y2[i], x0[i]);

                #ifdef BURNING_SHIP
                    INTRIN_CYCLE _2xy[i] = mm_and_ps(_2xy[i], abs_mask);
                #endif

                INTRIN_CYCLE y[i] = mm_add_ps(_2xy[i], y0[i]);
            }

            INTRIN_CYCLE {
                mXXXi * store_addr = (mXXXi *)(md->num_pixels + iy * sc_width + ix + i * NUMS_IN_PACK);
                mm_storeu_siXXX(store_addr, n[i]);
            }
        }
    }
}

static void * threadCalcMandelbrot(void * md_ptr)
{
    assert(md_ptr);

    mandelbrot_context_t * md = (mandelbrot_context_t *)md_ptr;
    calcMandelbrotConveyor(md);

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


#define PACK_CYCLE for (size_t i = 0; i < GCC_OPT_PACK_SIZE; i++)

void calcMandelbrotGCCoptimized(mandelbrot_context_t * md)
{
    const uint32_t sc_width  = md->sc_width;
    const uint32_t sc_height = md->sc_height;
    const uint32_t iter_num  = md->iter_num;

    const float left_x  = md->center_x - md->sc_width * md->scale / 2;
    const float right_x = md->sc_width * md->scale / 2 + md->center_x;

    const float bottom_y = md->center_y - md->sc_height * md->scale / 2;

    const float dx = (right_x - left_x) / md->sc_width;
    const float dy = dx;

    float delta[GCC_OPT_PACK_SIZE] = {};
    PACK_CYCLE delta[i] = dx * i;

    for (uint32_t iy = 0; iy < sc_height; iy++){
        float y0[GCC_OPT_PACK_SIZE] = {};
        PACK_CYCLE y0[i] = bottom_y + iy * dy;

        for (uint32_t ix = 0; ix < sc_width; ix += GCC_OPT_PACK_SIZE){
            float x0[GCC_OPT_PACK_SIZE] = {};
            PACK_CYCLE x0[i] = left_x + ix*dx + delta[i];

            float x[GCC_OPT_PACK_SIZE] = {};
            PACK_CYCLE x[i] = x0[i];

            float y[GCC_OPT_PACK_SIZE] = {};
            PACK_CYCLE y[i] = y0[i];

            int n[GCC_OPT_PACK_SIZE] = {0};

            for (uint32_t iteration = 0; iteration < iter_num; iteration++){
                float x2[GCC_OPT_PACK_SIZE] = {};
                PACK_CYCLE x2[i] = x[i] * x[i];

                float y2[GCC_OPT_PACK_SIZE] = {};
                PACK_CYCLE y2[i] = y[i] * y[i];

                float _2xy[GCC_OPT_PACK_SIZE] = {};
                PACK_CYCLE _2xy[i] = 2 * x[i] * y[i];

                uint32_t cmp_res[GCC_OPT_PACK_SIZE] = {};
                PACK_CYCLE cmp_res[i] = (x2[i] + y2[i] < MAX_R2);

                PACK_CYCLE n[i] += cmp_res[i];

                uint32_t mask = 0;
                PACK_CYCLE{
                    // mask <<= 1; //! uncommenting this line increases calc time x2
                    mask |= cmp_res[i];
                }

                if (!mask)
                    break;

                #ifdef BURNING_SHIP
                    PACK_CYCLE _2xy[i] = fabs(_2xy[i]);
                #endif

                PACK_CYCLE x[i] = x2[i] - y2[i] + x0[i];
                PACK_CYCLE y[i] = _2xy[i] + y0[i];
            }

            uint32_t * start_addr = md->num_pixels + iy * sc_width + ix;
            PACK_CYCLE start_addr[i] = n[i];
        }
    }
}

void calcMandelbrotNoOptimization(mandelbrot_context_t * md)
{
    const uint32_t sc_width  = md->sc_width;
    const uint32_t sc_height = md->sc_height;
    const uint32_t iter_num  = md->iter_num;

    const float left_x  = md->center_x - md->sc_width * md->scale / 2;
    const float right_x = md->sc_width * md->scale / 2 + md->center_x;

    const float bottom_y = md->center_y - md->sc_height * md->scale / 2;

    const float dx = (right_x - left_x) / md->sc_width;
    const float dy = dx;

    for (uint32_t iy = 0; iy < sc_height; iy++){
        float y0 = bottom_y + iy*dy;

        for (uint32_t ix = 0; ix < sc_width; ix++){
            float x0 = left_x + ix*dx;

            float x = x0;
            float y = y0;

            uint32_t n = 0;
            for (; n < iter_num; n++){
                float x2   = x * x;
                float y2   = y * y;
                float _2xy = 2 * x * y;

                if (x2 + y2 > MAX_R2)
                    break;

                x = x2 - y2 + x0;
                y = _2xy + y0;

            }

            md->num_pixels[iy * sc_width + ix] = n;
        }
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

static void calculateColorTable(const mandelbrot_context_t * md)
{
    assert(md);

    for (size_t col_index = 0; col_index < COLOR_TABLE_LEN; col_index++){
        md->color_table[col_index] = numToColor(col_index, md->iter_num);
    }
}

void numsToColor(const mandelbrot_context_t * md)
{
    assert(md);

    const uint32_t len = md->sc_height * md->sc_width;
    const uint32_t iter_num = md->iter_num;

    uint32_t * color_pixels = md->color_pixels;
    uint32_t * color_table  = md->color_table;

    uint32_t * num_pixels   = md->num_pixels;

    for (uint32_t num_index = 0; num_index < len; num_index++){
        if (num_pixels[num_index] == iter_num)
            color_pixels[num_index] = 0;
        else
            color_pixels[num_index] = color_table[num_pixels[num_index] % COLOR_TABLE_LEN];
    }
}
