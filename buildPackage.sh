#!/bin/bash

clang -Wall main.c -o package/game \
	/usr/local/lib/libraylib.a \
	/usr/local/lib/libzstd.a \
 	-I/usr/local/include \
	-framework CoreVideo \
	-framework IOKit \
	-framework Cocoa \
	-framework GLUT \
	-framework OpenGL \
	-lm \
	-O3

cp -r textures package
cp -r fonts package
cp -r sounds package
cp -r levels package

cp hiscore package

printf '\0\0\0\0\0\0\0\0' > package/hiscore

if [ $# -gt 0 ] && [ $1 == "nozip" ]; then
	echo "Package build complete without zipping"
else
	zip -r "Battle City 4000.zip" package
	echo "Package build complete and zipped"
fi