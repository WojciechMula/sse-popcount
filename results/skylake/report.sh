#!/bin/bash

name=skylake-i7-6700-gcc5.3.0-avx2

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst" \
    --cpu="Skylake Core i7-6700 CPU @ 3.40GHz" \
    --architecture="AVX2" \
    --compiler="GCC 5.3.0 (Ubuntu)" \
    --runs=5

if type rst2html > /dev/null
then
    echo "generating $name.html"
    rst2html "$name.rst" > "$name.html"
fi
