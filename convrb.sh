#!/bin/sh

if [ $# = 0 ]; then
	echo "Pass files to convert to GPC as arguments"
	exit 1
fi

loc=$(dirname "$0")
for file in "$@"; do
	filename=${file%.*}
	"$loc/convrb" < "$file" > "${filename}.GPC"
done

