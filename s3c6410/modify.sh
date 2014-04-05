#!/bin/sh
size=`stat -c "%s" $1`
outfile=skip

even=$(($size%2))

dd if=$1 of=$outfile bs=2 skip=16 conv=sync 2>/dev/null



echo "File:skip size:$(($size+$even-32)) created"
exit 0

