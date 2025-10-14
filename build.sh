#!/bin/bash
set -e

APP_NAME="Battle City 4000"
BUNDLE="$APP_NAME.app"

# statically link raylib from /usr/local/include and /usr/local/lib
clang -Wall main.c -mmacosx-version-min=15.0 \
  /usr/local/lib/libraylib.a \
  -I/usr/local/include \
  -framework CoreFoundation \
  -framework OpenGL \
  -framework Cocoa \
  -framework IOKit \
  -framework CoreVideo \
  -o "$BUNDLE/Contents/MacOS/bc4000"

echo "Build complete: $BUNDLE"

chmod +x "$BUNDLE/Contents/MacOS/bc4000" 

# resets highscore when building
printf '\0\0\0\0\0\0\0\0' > "$BUNDLE/Contents/Resources/hiscore"

./"$BUNDLE/Contents/MacOS/bc4000"