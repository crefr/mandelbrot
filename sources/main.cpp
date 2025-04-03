#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "mandelbrot.h"
#include "window_handler.h"
#include "test_mandelbrot.h"

const uint32_t SC_WIDTH  = 1280;
const uint32_t SC_HEIGHT = 720;

int main(int argc, char ** argv)
{
    if (argc > 1 && strcmp(argv[1], "-t") == 0){
        if (argc != 3){
            fprintf(stderr, "ERROR: -t num of cycles expected\n");
            return 1;
        }

        size_t measure_time = atoi(argv[2]);

        mandelbrot_context_t md = mandelbrotCtor(SC_WIDTH, SC_HEIGHT);

        printf("--------TESTING (%u x %u)--------\n", SC_WIDTH, SC_HEIGHT);
        testMandelbrot(&md, measure_time);

        return 0;
    }

    runWindow(SC_WIDTH, SC_HEIGHT);

    return 0;
}
