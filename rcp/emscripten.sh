#!/usr/bin/env sh

set -euo pipefail

# WARN: MUST BE CALLED FROM PROJECT ROOT DIRECTORY!

TMP="tmp/emscripten"
BIN="bin/emscripten"
EXE="$BIN/rlspr"
SRC="src"

which emcc > /dev/null || ( echo "Emscripten compiler is required in order to run this script!" && exit 1 )

mkdir -p $TMP $BIN

# https://github.com/raysan5/raylib/wiki/Working-for-Web-%28HTML5%29#21-command-line-compilation
RAYDIR=./lib/raylib/src
pushd $RAYDIR
make clean
# make
emcc -c rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c utils.c -Os -Wall -DPLATFORM_WEB
emcc -c raudio.c -Os -Wall -DPLATFORM_WEB
# emar rcs libraylib.a rcore.o rshapes.o rtextures.o rtext.o rmodels.o utils.o raudio.o
popd

emcc lib/raylib/src/*.o src/*.c -s USE_GLFW=3 -s ASYNCIFY -o index.html

rm *.zip -f
zip rlspr.zip index*

# for i in $(ls $SRC | grep '.c'); do
# 	emcc $SRC/$i -c -o $TMP/$i.o
# done

