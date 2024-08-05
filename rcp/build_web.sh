#!/usr/bin/env sh
# WARN: MUST BE CALLED FROM PROJECT ROOT DIRECTORY!

set -euo pipefail

TMP="tmp/emcc"
BIN="bin/emcc"
SRC="src"
RCP="rcp"
RAYSRC=lib/raylib/src

for i in emcc zip; do
	which $i > /dev/null || exit 1
done

mkdir -p $TMP $BIN

# https://github.com/raysan5/raylib/wiki/Working-for-Web-%28HTML5%29#21-command-line-compilation
pushd $RAYSRC
make clean
emcc -c rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c utils.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c raudio.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
popd

mv $RAYSRC/*.o $TMP

emcc $TMP/*.o $SRC/*.c --shell-file $RCP/shell.html -s USE_GLFW=3 -s ASYNCIFY -o $BIN/index.html

zip -u $BIN/rlspr.zip $BIN/index*

