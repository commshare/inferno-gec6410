#!/bin/sh
#use to cut off the header(used to define a executable profile) and make up the end(when size is even).
size=`stat -c "%s" $1`
outfile=skip

even=$(($size%2))

dd if=$1 of=$outfile bs=2 skip=16 conv=sync 2>/dev/null



echo "File:skip size:$(($size+$even-32)) created"
exit 0

