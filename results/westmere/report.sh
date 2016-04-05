#!/bin/bash

name=westmere-m540-gcc4.9.2-sse

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst"

if type rst2html > /dev/null
then
    echo "generating $name.html"
    rst2html "$name.rst" > "$name.html"
fi
