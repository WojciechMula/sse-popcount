#!/bin/bash

name=bulldozer-fx-8510-gcc4.8.4-sse

python ../../scripts/report.py \
    --csv="$name.csv" \
    --output="$name.rst"

if type rst2html > /dev/null
then
    echo "generating $name.html"
    rst2html "$name.rst" > "$name.html"
fi
