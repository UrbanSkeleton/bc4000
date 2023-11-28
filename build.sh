#!/bin/bash

clang -arch x86_64 -Wall main.c -o bc4000 -l raylib && ./bc4000
