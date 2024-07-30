#!/usr/bin/env sh
RDIR='./lib/raylib/src'
SRCS=$(ls $RDIR | grep '\.c')
HDRS=$(ls $RDIR | grep '\.h')
for i in $HDRS; do
	cp -f $RDIR/$i ./$i
done
for i in $SRCS; do
	# echo
	# echo "==== $i ======"
	# cat $i | grep '#include'
	cp -f $RDIR/$i ./$i
	cc $i -DPLATFORM_DESKTOP -c -o $i.o
done
