#!/bin/bash

prog="../../speed_arm_g++ --csv"
iters=5
tmp=tmp.csv
result=result.csv

rm -f $tmp
for i in `seq $iters`
do
    echo "run $i"
    $prog   32 10000000 | tee -a $tmp
    $prog   64  5000000 | tee -a $tmp
    $prog  128  2500000 | tee -a $tmp
    $prog  256  1250000 | tee -a $tmp
    $prog  512  1000000 | tee -a $tmp
    $prog 1024   500000 | tee -a $tmp
    $prog 2048   250000 | tee -a $tmp
    $prog 4096   125000 | tee -a $tmp
done

mv $tmp $result
