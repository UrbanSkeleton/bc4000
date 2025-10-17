#!/bin/bash

clang -Wall main.c -o bc4000 -l raylib -l zstd && ./bc4000
