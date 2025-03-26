#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mandelbrot.h"
#include "window_handler.h"

const uint32_t SC_WIDTH  = 2560;
const uint32_t SC_HEIGHT = 1440;

int main()
{
    runWindow(SC_WIDTH, SC_HEIGHT);

    return 0;
}
