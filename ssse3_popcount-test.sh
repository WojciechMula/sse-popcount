tmp=`tempfile`

for size in 1 8 32; do
	for proc in lookup "ssse3-1" "ssse3-2"; do
		/usr/bin/time -o "$tmp" -f "%U" ./ssse3_popcount $proc $size 2000000
		echo -n $proc, $size, " "
		cat $tmp
	done
done
for size in 128 512 1024 2048; do
	for proc in lookup "ssse3-1" "ssse3-2"; do
		/usr/bin/time -o "$tmp" -f "%U" ./ssse3_popcount $proc $size 200000
		echo -n $proc, $size, " "
		cat $tmp
	done
done
