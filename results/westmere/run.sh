#!/bin/bash

prog="../../speed --csv"
iters=5
tmp=tmp.csv
result=result.csv

rm -f $tmp
for i in `seq $iters`
do
    echo "run $i"
    $prog   32 100000000 | tee -a $tmp
    $prog   64  50000000 | tee -a $tmp
    $prog  128  25000000 | tee -a $tmp
    $prog  256  12500000 | tee -a $tmp
    $prog  512  10000000 | tee -a $tmp
    $prog 1024   5000000 | tee -a $tmp
    $prog 2048   2500000 | tee -a $tmp
    $prog 4094   1250000 | tee -a $tmp
done

mv $tmp $result
