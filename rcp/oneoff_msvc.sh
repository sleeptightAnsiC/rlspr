#!/usr/bin/env sh

# builds source tree with MSVC compiler (cl.exe) and runs it
# WARN: Make sure to call this script from mtpl root directory!
# WARN: This will fail while being outside of WindowsNT without wine

set -euo pipefail

CFLAGS="$@"
TMP="tmp/msvc"
BIN="bin/msvc"
EXE="$BIN/mtpl.exe"
SRC="src"

which cl > /dev/null || ( echo "MSVC compiler (cl.exe) is required in order to run this script!" && exit 1 )

mkdir -p $TMP $BIN

for i in $(ls $SRC | grep '.c'); do
	cl $SRC/$i /c /Fo: $TMP/$i.obj $CFLAGS
done

OBJS=()
for i in $(ls $TMP | grep '.obj'); do
	OBJS+=("$TMP/$i")
done
cl ${OBJS[@]} /Fe: $EXE $CFLAGS

$(which "wine") ./$EXE

