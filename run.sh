size=1000000
iter=1000
names="cpu sse-lookup avx2-lookup sse-bit-parallel bit-parallel-optimized bit-parallel lookup-8 lookup-64"

for name in $names
do
    ./$1 $name $size $iter
done
