#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "mandelbrot.h"
#include "window_handler.h"

const uint32_t SC_WIDTH  = 1280;
const uint32_t SC_HEIGHT = 720;

int main(int argc, char ** argv)
{
    if (argc > 1 && strcmp(argv[1], "-t") == 0){
        if (argc != 3){
            fprintf(stderr, "ERROR: -t num of cycles expected\n");
            return 1;
        }

        size_t num_of_cycles = atoi(argv[2]);
        uint32_t * pixels = (uint32_t *)calloc(SC_WIDTH * SC_HEIGHT, sizeof(*pixels));

        clock_t start_time = clock();
        for (size_t cycle_index = 0; cycle_index < num_of_cycles; cycle_index++){
            calcCenteredMandelbrot(pixels, SC_WIDTH, SC_HEIGHT, -0.5, 0., 2./(SC_WIDTH));
        }
        clock_t end_time = clock();

        printf("one frame mean calc time = %lf ms\n", (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000 / num_of_cycles);

        return 0;
    }

    runWindow(SC_WIDTH, SC_HEIGHT);

    return 0;
}
