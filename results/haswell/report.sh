#!/bin/bash

name=haswell-i7-4770-gcc5.3.0-avx2

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst"

name=haswell-i7-4770-clang3.8.0-avx2

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst"

if type rst2html > /dev/null
then
    for src in *.rst
    do
        dst=`basename $src .rst`.html
        rst2html "$src" > "$dst"
    done
fi
