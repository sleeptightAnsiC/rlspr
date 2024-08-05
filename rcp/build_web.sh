#!/usr/bin/env sh

set -euo pipefail

# WARN: MUST BE CALLED FROM PROJECT ROOT DIRECTORY!

TMP="tmp/emscripten"
BIN="bin/emscripten"
SRC="src"
RAYDIR=./lib/raylib/src

for i in emcc zip; do
	which $i > /dev/null || exit 1
done

mkdir -p $TMP $BIN

# https://github.com/raysan5/raylib/wiki/Working-for-Web-%28HTML5%29#21-command-line-compilation
pushd $RAYDIR
make clean
emcc -c rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c utils.c -Os -Wall -DPLATFORM_WEB
emcc -c raudio.c -Os -Wall -DPLATFORM_WEB
popd

mv $RAYDIR/*.o $TMP

emcc $TMP/*.o src/*.c --shell-file shell.html -s USE_GLFW=3 -s ASYNCIFY -o index.html

rm *.zip -f
zip rlspr.zip index*

