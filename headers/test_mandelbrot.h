#ifndef TEST_MANDELBROT_INCLUDED
#define TEST_MANDELBROT_INCLUDED

#include "mandelbrot.h"

typedef struct {
    double time;
    double sigma;
} test_result_t;

/// @brief test mandelbrot calculating func and returns one frame mean time and its error (sigma)
test_result_t testMandelbrotFunc(void (*mandelFunction)(mandelbrot_context_t * md),  mandelbrot_context_t * md, const size_t measure_time);

/// @brief tests different calculating functions and prints result into stdout
void testMandelbrot(mandelbrot_context_t * md, const size_t measure_time);

/// @brief shell for calcMandelbrotMultithread for prototype unification
void calcMandelbrot8Threads(mandelbrot_context_t * md);

/// @brief prints main information about this session
void printOptionsInfo(mandelbrot_context_t * md);

#endif
