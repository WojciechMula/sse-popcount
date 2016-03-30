#!/bin/bash

for dir in haswell skylake westmere sandybridge-e
do
    cd $dir
    ./report.sh
    cd ..
done
