#!/usr/bin/env sh
# WARN: MUST BE CALLED FROM PROJECT ROOT DIRECTORY!

# https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)

set -euo pipefail

SRCDIR="src"
RCPDIR="rcp"
RAYDIR="raylib/src"

TMPDIR="tmp/emcc"
BINDIR="bin/emcc"
PKGDIR="pkg/emcc"
mkdir -p $TMPDIR $BINDIR $PKGDIR

which emcc > /dev/null || exit 1

# FIXME:
# I have no idea why everything breaks when I put the code below into forloop
# or when I try to build it outside of raylib/src directory
# Perhaps compiler links/includes something I don't know about implicitly...
# INCREMENT THE COUNTER EACH TIME YOU FAILED TO FIX THIS SH1T: 3

pushd $RAYDIR
make clean
emcc -c rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c utils.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c raudio.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
popd

mv $RAYDIR/*.o $TMPDIR

emcc $TMPDIR/*.o $SRCDIR/*.c --shell-file $RAYDIR/minshell.html -s USE_GLFW=3 -s ASYNCIFY -o $BINDIR/index.html -sALLOW_MEMORY_GROWTH

which zip > /dev/null || exit 1
zip -u $PKGDIR/rlspr.zip $BINDIR/index*

