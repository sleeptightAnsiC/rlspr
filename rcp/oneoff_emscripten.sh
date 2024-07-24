#!/usr/bin/env sh

# builds source tree with emscripten compiler and runs with nodejs
# WARN: Make sure to call this script from mtpl root directory!

set -euo pipefail

CFLAGS="$@"
TMP="tmp/emscripten"
BIN="bin/emscripten"
EXE="$BIN/mtpl"
SRC="src"

which emcc > /dev/null || ( echo "Emscripten compiler is required in order to run this script!" && exit 1 )
which node > /dev/null || ( echo "NodeJS is required in order to run this script!" && exit 1 )

mkdir -p $TMP $BIN

for i in $(ls $SRC | grep '.c'); do
	emcc $SRC/$i $CFLAGS -c -o $TMP/$i.o
done

OBJS=()
for i in $(ls $TMP | grep '.o'); do
	OBJS+=("$TMP/$i")
done
emcc ${OBJS[@]} $CFLAGS -o $EXE

node $EXE

