#!/bin/bash

clang -Wall main.c -o package/game -I/usr/local/include -l raylib

cp -r textures package
cp -r fonts package
cp -r sounds package
cp -r levels package

cp hiscore package

printf '\0\0\0\0\0\0\0\0' > package/hiscore

if [ $1 != "nozip" ]; then
	zip -r "Battle City 4000.zip" package
	echo "Package build complete and zipped"
else
	echo "Package build complete without zipping"
fi

