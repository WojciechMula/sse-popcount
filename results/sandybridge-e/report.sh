#!/bin/bash
name=sandybridgeE-i7-3930k-g++4.8-avx

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst" \
    --cpu="SandyBridge-E Core i7-3930k CPU @ 3.2GHz " \
    --architecture="AVX" \
    --compiler="GCC 4.8.5 (Ubuntu)" \
    --runs=5


name=sandybridgeE-i7-3930k-g++5.3-avx
python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst" \
    --cpu="SandyBridge-E Core i7-3930k CPU @ 3.2GHz " \
    --architecture="AVX" \
    --compiler="GCC 5.3.0 (Ubuntu)" \
    --runs=5

if type rst2html > /dev/null
then
    for src in *.csv
    do
        dst=`basename $src .csv`.html
        rst2html "$src" > "$dst"
    done
fi
