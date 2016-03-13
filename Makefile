CC=g++
FLAGS=-std=c++11 -mpopcnt -O2 -Wall -pedantic -Wextra

DEPS=popcnt-*.cpp config.h
ALL=speed verify
ALL_AVX2=speed_avx2 verify_avx2

.PHONY: all avx2

all: $(ALL)

avx2: $(ALL_AVX2)

speed: $(DEPS) speed.cpp
	$(CC) $(FLAGS) -mssse3 speed.cpp -o $@

verify: $(DEPS) verify.cpp
	$(CC) $(FLAGS) -mssse3 verify.cpp -o $@

speed_avx2: $(DEPS) speed.cpp
	$(CC) $(FLAGS) -mavx2 -DHAVE_AVX2_INSTRUCTIONS speed.cpp -o $@

verify_avx2: $(DEPS) verify.cpp
	$(CC) $(FLAGS) -mavx2 -DHAVE_AVX2_INSTRUCTIONS verify.cpp -o $@

SIZE=10000000
ITERS=1000

run: speed
	./speed $(SIZE) $(ITERS)

run_avx2: speed_avx2
	./speed_avx2 $(SIZE) $(ITERS)

clean:
	rm -f $(ALL) $(ALL_AVX2)
