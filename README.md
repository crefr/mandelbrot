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


