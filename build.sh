#!/bin/bash

clang -Wall main.c -o bc4000 -I/usr/local/include -l raylib && ./bc4000
