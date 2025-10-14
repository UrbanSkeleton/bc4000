#!/bin/bash
set -e

APP_NAME="Battle City 4000"
BUNDLE="$APP_NAME.app"

clang -Wall main.c -o "$BUNDLE/Contents/MacOS/bc4000" \
  -I/usr/local/include \
  -L/usr/local/lib \
  -lraylib \
  -framework CoreFoundation \
  -framework OpenGL \
  -framework Cocoa \
  -framework IOKit \
  -framework CoreVideo

echo "Build complete: $BUNDLE"

chmod +x "$BUNDLE/Contents/MacOS/bc4000" 

printf '\0\0\0\0\0\0\0\0' > "$BUNDLE/Contents/Resources/hiscore"

./"$BUNDLE/Contents/MacOS/bc4000"