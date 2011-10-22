# usage: sh ssse3_popcount.sh 2> ssse3_popcount_speedup.data

for size in 1 8; do
	for proc in lookup "sse2-1" "sse2-2" "ssse3-1" "ssse3-2"; do
		/usr/bin/time -f "$proc $size %U" ./ssse3_popcount $proc $size 20000000
	done
done
for size in 32; do
	for proc in lookup "sse2-1" "sse2-2" "ssse3-1" "ssse3-2" "sse2-unrl" "ssse3-unrl"; do
		/usr/bin/time -f "$proc $size %U" ./ssse3_popcount $proc $size 2000000
	done
done
for size in 128 512 1024 2048; do
	for proc in lookup "sse2-1" "sse2-2" "ssse3-1" "ssse3-2" "sse2-unrl" "ssse3-unrl"; do
		/usr/bin/time -f "$proc $size %U" ./ssse3_popcount $proc $size 200000
	done
done
