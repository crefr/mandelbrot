#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "test_mandelbrot.h"
#include "mandelbrot.h"

test_result_t testMandelbrot(void (*mandelFunction)(mandelbrot_context_t * md),  mandelbrot_context_t * md, const size_t num_of_cycles)
{
    double sum_of_T  = 0;
    double sum_of_T2 = 0;

    for (size_t cycle_index = 0; cycle_index < num_of_cycles; cycle_index++){
        struct timespec calc_start = {};
        struct timespec calc_end = {};

        clock_gettime(CLOCK_MONOTONIC, &calc_start);
        mandelFunction(md);
        clock_gettime(CLOCK_MONOTONIC, &calc_end);

        double test_time = 1000 * (calc_end.tv_sec - calc_start.tv_sec) + (calc_end.tv_nsec - calc_start.tv_nsec) / 1e6;

        sum_of_T  += test_time;
        sum_of_T2 += test_time * test_time;
    }

    double mean_T  = sum_of_T / num_of_cycles;
    double mean_T2 = sum_of_T2 / num_of_cycles;

    double sigma_T = sqrt((mean_T2 - mean_T * mean_T) / num_of_cycles);

    test_result_t result = {
        .time  = mean_T,
        .sigma = sigma_T
    };

    return result;
}

void calcMandelbrot8Threads(mandelbrot_context_t * md)
{
    calcMandelbrotMultiThread(md, 8);
}
