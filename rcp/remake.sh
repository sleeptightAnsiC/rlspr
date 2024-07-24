#!/usr/bin/env sh

# https://superuser.com/a/665208

if command -v entr &> /dev/null; then
	while sleep 1; do
		find src tmp rcp Makefile | entr -dcc make $@
	done
else
	echo "Error: entr is required in order to run this script!"
	exit 1
fi;

