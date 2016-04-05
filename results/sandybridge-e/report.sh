#!/bin/bash
name=sandybridgeE-i7-3930k-g++4.8-avx

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst"

name=sandybridgeE-i7-3930k-g++5.3-avx
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
