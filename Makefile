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

run: speed
	sh run.sh speed

run_avx2: speed_avx2
	sh run.sh speed_avx2

clean:
	rm -f $(ALL) $(ALL_AVX2)
