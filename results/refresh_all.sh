#!/bin/bash

for dir in haswell skylake westmere
do
    cd $dir
    ./report.sh
    cd ..
done
