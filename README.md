# Mandelbrot

This project is the experiment of optimization using x86-64 SIMD instructions. So you need x86-64 processor if you want to try it on.

![Illustration 1](https://github.com/crefr/mandelbrot/raw/master/images/image_1.png)

![Illustration 2](https://github.com/crefr/mandelbrot/raw/master/images/image_2.png)


## Installation

Clone this repository:

```bash
git clone https://github.com/crefr/mandelbrot
```

Also you need [SFML library](https://www.sfml-dev.org/) of version 2.x.x (2.5.1 or newer).

In the folder with cloned repository use `make` to compile it:

```bash
make
```

In this folder should appear executable file `mandelbrot`.

## Launching

As the program is compiled you can use it.
You can use it normal way:

```bash
./mandelbrot
```

Or you can measure its calculating speed by adding flag `-t`, after there should be time of testing (in ms). For example:

```bash
./mandelbrot -t 5000
```
This command launches program in testing mode, so it will test different Mandelbrot calculating functions for 5 seconds each.

## Testing mode
### Description
Measurements were conducted in different modes:

#### 1. No optimizations
Base version without any optimizations.

#### 2. Compiler auto vectorization
Instead of calculating one pixel by one iteration of the deepest loop, we are calculating a pack of pixels (8, 16, 32 or 64). So compiler can see independent operations better and it is more likely optimizing it by SIMD instructions.

#### 3. SIMD
Optimizations by using SIMD intrinsics (SSE or AVX/AVX2).

#### 4. SIMD with conveyor
The same as SIMD but divided into bigger packs so we can independently use a couple of intrinsics at one pack. It is more effective because in this case there are more independent instructions in a row, so CPU's conveyor is used more effectively.

#### 5. SIMD w/conveyor threaded
There is a naive (bad) realiasation of multithreading. But it also gives some perfomance increase.


### Benchmark results on a 4-core Intel Core i5-8250U
Here are some performance benchmark results that were received by testing program on the computer with a 4-core Intel Core i5-8250U processor. Every test was lasting around 5 seconds. All measurements in the table are in one-thread mode. You can do similar benchmark by running program in testing mode that was decribed above.

Program was compiled with g++  11.4.0 compiler with following options:

+ Screen size - 1284x720
+ Auto vectorization pack size - 32
+ AVX/AVX2 SIMD instructions are used (not SSE)
+ SIMD instructions pack size (in SIMD conveyor) - 3


| Testing mode                      | Result, ms         | Relative boost    |
| ------------                      | ------             | ---               |
| No optimizations                  | $ 602 \pm 3 $      |  x 1              |
| Compiler auto vectorization       | $ 49.6 \pm 0.2 $   |  x 12.1           |
| SIMD                              | $ 82.8 \pm 0.1 $   |  x 7.3            |
| SIMD with conveyor                | $ 35.2 \pm 0.1 $   |  **x 17.1**       |

Also multithread version was tested with 8 threads but its increase relatively to one-thread "SIMD with conveyor" is only 2.9 times because of the bas realisation.

### Analysis
SIMD instructions increased performance by 7.3 times as expected (ymm register size is 256 bit = 8 floats). But with more effective conveyorization result is better by 2.4 times more.

Compiler auto vectorization is better than SIMD version also because of the better conveyorization.

**Conclusion:** "SIMD with conveyor" version gave the best results among one-threaded versions - its performance increase is **17.1 times**.

## Controls
### Position control:
`Esc` - return to the start position;

`F6`  - save current position to the file 'position.txt';

`F9`  - load position from file 'position.txt';

### Moving:

`W` / `UpArrow`    - move up;

`S` / `DownArrow`  - move down;

`A` / `LeftArrow`  - move left;

`D` / `RightArrow` - move right;

### Scaling:
`Mouse wheel up` - more zoom;

`Mouse wheel down` - less zoom;


## Additional options
### Parameters at [`headers/mandelbrot.h`](headers/mandelbrot.h)
1. You can turn AVX off and use only SSE by commenting this string at :
    ```c
    #define AVX_ON
    ```
2. By editing this parameter you can change compiler auto vectorization size:
    ```c
    #define GCC_OPT_PACK_SIZE 32
    ```

3. Here you can change SIMD pack size (in SIMD with conveyor mode):
    ```c
    #define INTRIN_PACK_SIZE 3
    ```
4. You can display Burning Ship fractal instead of Mandelbrot set - uncomment this line:
    ```c
    #define BURNING_SHIP
    ```

### Changing screen size
In [`sources/main.cpp`](sources/main.cpp) you can change screen size:

```c
const uint32_t SC_WIDTH  = 1284;
const uint32_t SC_HEIGHT = 720;
```

But make sure that `SC_WIDTH` is completely divisible by `INTRIN_PACK_SIZE` * pack_size, where pack_size is number of floats in register in chosen mode (8 for AVX, 4 for SSE).

