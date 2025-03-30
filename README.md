# Mandelbrot

This program creates window that displays Mandelbrot set. It is optimized
by using SSE/AVX intrinsics.

![Illustration 1](https://github.com/crefr/mandelbrot/raw/master/images/image_1.png)

![Illustration 2](https://github.com/crefr/mandelbrot/raw/master/images/image_2.png)


## Installation

Clone this repository:

```bash
git clone https://github.com/crefr/mandelbrot
```
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

Or you can measure its calculating speed by adding flag `-t` with num of tests you want:

```bash
./mandelbrot -t 200
```

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


## Additional
### Only SSE
You can turn AVX off and use only SSE by commenting this string at `sources/mandelbrot.cpp`:

```c
#define AVX_ON
```

and the compiling it again.

### Burning ship
You can display Burning Ship fractal instead of Mandelbrot set - uncomment this line at `sources/mandelbrot.cpp`:
```c
#define BURNING_SHIP
```

### GCC auto vectorisation
At the branch [`gcc_optimized`](https://github.com/crefr/mandelbrot/tree/gcc_optimized) there is a version of the program with dividing calculations into small cycles for better optimisation by `gcc`. It can see where to insert SSE/AVX instructions.

### No optimisation
At the branch [`no_optimize`](https://github.com/crefr/mandelbrot/tree/no_optimize) you can find version of the program without any optimisations.
