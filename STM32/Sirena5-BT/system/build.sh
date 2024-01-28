#!/bin/sh

BUILD_FILE="$(dirname "$0")/build.txt"
BUILD=$(expr $(head -n 1 "$BUILD_FILE") + 1)

if [ "$1" == "inc" ]; then
	echo $BUILD >"$BUILD_FILE"
elif [ "$1" == "full" ]; then
	echo $(date '+%Y-%m-%d') build $BUILD
else
	echo $BUILD
fi

exit 0
