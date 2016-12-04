#!/bin/bash

for dir in haswell skylake westmere sandybridge-e bulldozer
do
    cd $dir
    ./report.sh
    cd ..
done
