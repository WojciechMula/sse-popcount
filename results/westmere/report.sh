#!/bin/bash

name=westmere-m540-gcc4.9.2-sse

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst" \
    --cpu="Core i5 M540 @ 2.53GHz" \
    --architecture="SSE" \
    --compiler="GCC 4.9.2 (Debian 4.9.2-10)" \
    --runs=5

if type rst2html > /dev/null
then
    echo "generating $name.html"
    rst2html "$name.rst" > "$name.html"
fi
