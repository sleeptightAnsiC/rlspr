#!/usr/bin/env sh

# runs 'make build' with compilers defined in 'rcp/compilers.txt'

# BUG: (not worth fixing)
# compilers that build executable for WindowsNT will always add '.exe' to the name
# because of this, make will always try to recompile said files.

COMPILERS=$(cat rcp/compilers.txt)
cc_skipped=

echo "SUPERBUILD START"

echo
echo "SUPERBUILD BEGIN WARMUP"
make compile_commands.json tmp bin
echo "SUPERBUILD END WARMUP"

for i in $COMPILERS
do
	echo
	if [[ $(which $i) ]]
	then
		echo "SUPERBUILD BEGIN $i"
		make build CFLAGS="" CC="$i"
		echo "SUPERBUILD END $i"
	else
		echo "SUPERBUILD CANNOT FIND $i"
		cc_skipped+=" $i"
	fi
done

echo
if [[ $cc_skipped ]]
then
	echo "SUPERBUILD WARNING! some compilers could not be found and were skipped:$cc_skipped"
fi
echo "SUPERBUILD DONE"

