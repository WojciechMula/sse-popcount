.PHONY: all build_all x86 avx avx2 help speed verify speed_avx verify_avx speed_avx2 verify_avx2

# user can do CXX=g++ make
# It's more fexlible to change from command line
# The make builtin rule states CXX to be g++.

COMPILER=$(notdir $(CXX))
FLAGS=-std=c++11 -mpopcnt -O2 -Wall -pedantic -Wextra

DEPS=popcnt-*.cpp function_registry.cpp config.h
ALL=speed_$(COMPILER) verify_$(COMPILER)
ALL_AVX=speed_avx_$(COMPILER) verify_avx_$(COMPILER)
ALL_AVX2=speed_avx2_$(COMPILER) verify_avx2_$(COMPILER)
ALL_TARGETS=$(ALL) $(ALL_AVX) $(ALL_AVX2)

all: $(ALL)

help:
	@echo "targets:"
	@echo "x86      - makes programs verify & speed (the default target)"
	@echo "avx      - makes programs verify_avx & speed_avx"
	@echo "avx2     - makes programs verify_avx2 & speed_avx2"
	@echo "run      - runs speed test for x86 code"
	@echo "run_avx  - runs speed test for AVX code"
	@echo "run_avx2 - runs speed test for AVX2 code"


x86: $(ALL)

avx: $(ALL_AVX)

avx2: $(ALL_AVX2)

speed_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS) -mssse3 speed.cpp -o $@

verify_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS) -mssse3 verify.cpp -o $@

speed_avx_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS) -mavx speed.cpp -o $@

verify_avx_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS) -mavx verify.cpp -o $@

speed_avx2_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS) -mavx2 -DHAVE_AVX2_INSTRUCTIONS speed.cpp -o $@

verify_avx2_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS) -mavx2 -DHAVE_AVX2_INSTRUCTIONS verify.cpp -o $@

speed: speed_$(COMPILER)
speed_avx: speed_avx_$(COMPILER)
speed_avx2: speed_avx2_$(COMPILER)

verify: verify_$(COMPILER)
verify_avx: verify_avx_$(COMPILER)
verify_avx2: verify_avx2_$(COMPILER)


build_all: $(ALL_TARGETS)

SIZE=10000000
ITERS=100

run: speed
	./speed_$(COMPILER) $(SIZE) $(ITERS)

run_avx: speed_avx
	./speed_avx_$(COMPILER) $(SIZE) $(ITERS)

run_avx2: speed_avx2
	./speed_avx2_$(COMPILER) $(SIZE) $(ITERS)

clean:
	rm -f $(ALL_TARGETS)
