#ifndef TEST_MANDELBROT_INCLUDED
#define TEST_MANDELBROT_INCLUDED

#include "mandelbrot.h"

typedef struct {
    double time;
    double sigma;
} test_result_t;

test_result_t testMandelbrot(void (*mandelFunction)(mandelbrot_context_t * md),  mandelbrot_context_t * md, const size_t num_of_cycles);

#endif
