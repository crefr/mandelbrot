#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#include "test_mandelbrot.h"
#include "mandelbrot.h"

test_result_t testMandelbrotFunc(void (*mandelFunction)(mandelbrot_context_t * md),  mandelbrot_context_t * md, const size_t measure_time)
{
    assert(mandelFunction);
    assert(md);

    double sum_of_T  = 0;
    double sum_of_T2 = 0;
    size_t cycle_num = 0;

    while (sum_of_T < measure_time){
        struct timespec calc_start = {};
        struct timespec calc_end = {};

        clock_gettime(CLOCK_MONOTONIC, &calc_start);
        mandelFunction(md);
        clock_gettime(CLOCK_MONOTONIC, &calc_end);

        double test_time = 1000 * (calc_end.tv_sec - calc_start.tv_sec) + (calc_end.tv_nsec - calc_start.tv_nsec) / 1e6;

        sum_of_T  += test_time;
        sum_of_T2 += test_time * test_time;
        cycle_num++;
    }

    double mean_T  = sum_of_T / cycle_num;
    double mean_T2 = sum_of_T2 / cycle_num;

    double sigma_T = sqrt((mean_T2 - mean_T * mean_T) / cycle_num);

    test_result_t result = {
        .time  = mean_T,
        .sigma = sigma_T
    };

    return result;
}

static void printFuncTime(void (*mandelFunction)(mandelbrot_context_t * md), mandelbrot_context_t * md, const size_t measure_time)
{
    test_result_t time = testMandelbrotFunc(mandelFunction, md, measure_time);
    printf("mean calc time = (%lf +- %lf) ms\n", time.time, time.sigma);
}

void testMandelbrot(mandelbrot_context_t * md, const size_t measure_time)
{
    struct test_func {
        const char * func_name;
        void (*mandelFunction)(mandelbrot_context_t * md);
    };

    const struct test_func tests[] = {
        {"NON-OPTIMIZED VERSION", calcMandelbrotNoOptimization  },
        {"COMPILER OPTIMIZATION", calcMandelbrotGCCoptimized    },
        {"INTRINSICS"           , calcMandelbrot                },
        {"INTRINSICS + CONVEYOR", calcMandelbrotConveyor        },
        {"INTRINSICS 8 THREADS ", calcMandelbrot8Threads        }
    };
    const size_t test_num = sizeof(tests) / sizeof(*tests);

    for (size_t test_index = 0; test_index < test_num; test_index++){
        printf("%s\n", tests[test_index].func_name);
        printFuncTime(tests[test_index].mandelFunction, md, measure_time);
        printf("\n");
    }
}

void calcMandelbrot8Threads(mandelbrot_context_t * md)
{
    calcMandelbrotMultiThread(md, 8);
}
