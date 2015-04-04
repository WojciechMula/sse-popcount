CC=g++
FLAGS=-std=c++11 -mssse3 -O2 -Wall -pedantic -Wextra

DEPS=popcnt-*.cpp
ALL=speed verify

.PHONY: all

all: $(ALL)

speed: $(DEPS) speed.cpp
	$(CC) $(FLAGS) speed.cpp -o speed

verify: $(DEPS) verify.cpp
	$(CC) $(FLAGS) verify.cpp -o verify

run: speed
	sh run.sh speed

clean:
	rm -f $(ALL)
