#!/bin/bash

for dir in *
do
    if [[ -d ${dir} ]]; then
        ./report.sh ${dir}
    fi
done
