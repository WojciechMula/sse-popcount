size=1000000
iter=1000
names="cpu lookup-8 lookup-64 bit-parallel bit-parallel-optimized sse-bit-parallel sse-lookup"

for name in $names
do
    ./$1 $name $size $iter
done
