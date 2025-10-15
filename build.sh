#!/bin/bash

clang -Wall main.c -mmacosx-version-min=15.0 \
  /usr/local/lib/libraylib.a \
  -I/usr/local/include \
  -o "./bc4000"

# resets highscore when building
printf '\0\0\0\0\0\0\0\0' > "./hiscore"

./bc4000