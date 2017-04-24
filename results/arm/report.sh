#!/bin/bash

name=armv7-32bit-gcc4.9.2

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst"

name=arm-64bit-gcc4.8.5

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst"

if type rst2html > /dev/null
then
    for src in *.rst
    do
        echo "generating $src.html"
        dst=`basename $src .rst`.html
        rst2html "$src" > "$dst"
    done
fi
