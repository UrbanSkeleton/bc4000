This is a clone of NES game Battle City, written in C using [raylib](https://www.raylib.com) library.

## Rationale

Nah, I did it just for fun.

## Build:

First, install raylib. Then

```
./build.sh
```

or

```
clang main.c -o bc4000 -l raylib && ./bc4000
```

Only tested on MacOS, but should work on Linux and maybe even Windows, as the only dependency is raylib.

To build with alternative assets and soundtrack from [UrbanSkeleton](https://scratch.mit.edu/users/UrbanSkeleton/), uncomment `#define ALT_ASSETS` line in main.c.

## Controls:

Player 1: w/a/s/d + `space` to fire.

Player 2: arrow keys + `,` to fire.

`left shift` to switch mode.

`enter` to select.

`cmd+Q` to exit.

Enjoy.
