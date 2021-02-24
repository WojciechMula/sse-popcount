#!/bin/bash

prog=$1
iters=5
tmp=tmp.csv
result=result.csv

rm -f $tmp
for i in `seq $iters`
do
    echo "run $i"
    taskset -c 0 $prog --csv   32 100000000 | tee -a $tmp
    taskset -c 0 $prog --csv   64  50000000 | tee -a $tmp
    taskset -c 0 $prog --csv  128  25000000 | tee -a $tmp
    taskset -c 0 $prog --csv  256  12500000 | tee -a $tmp
    taskset -c 0 $prog --csv  512  10000000 | tee -a $tmp
    taskset -c 0 $prog --csv 1024   5000000 | tee -a $tmp
    taskset -c 0 $prog --csv 2048   2500000 | tee -a $tmp
    taskset -c 0 $prog --csv 4096   1250000 | tee -a $tmp
done

mv $tmp $result
