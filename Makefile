.PHONY: all build-all x86 avx avx2 help speed verify speed_avx verify_avx speed_avx2 verify_avx2

# user can do CXX=g++ make
# It's more fexlible to change from command line
# The make builtin rule states CXX to be g++.

COMPILER=$(notdir $(CXX))
FLAGS=-std=c++11 -O2 -Wall -pedantic -Wextra -Wfatal-errors
SDE=sde # path to the Intel Software Development Emulator, see:
        # https://software.intel.com/en-us/articles/intel-software-development-emulator
FLAGS_INTEL=$(FLAGS) -mpopcnt -fabi-version=6
FLAGS_ARM=$(FLAGS) -mfpu=neon -DHAVE_NEON_INSTRUCTIONS

FLAGS_SSE=$(FLAGS_INTEL) -mssse3 -DHAVE_SSE_INSTRUCTIONS
FLAGS_AVX=$(FLAGS_INTEL) -mavx -DHAVE_AVX_INSTRUCTIONS
FLAGS_AVX2=$(FLAGS_INTEL) -mavx2 -DHAVE_AVX_INSTRUCTIONS
FLAGS_AVX512BW=$(FLAGS_INTEL) -mavx512bw -DHAVE_AVX512BW_INSTRUCTIONS

DEPS=popcnt-*.cpp function_registry.cpp sse_operators.cpp config.h
ALL=speed_$(COMPILER) verify_$(COMPILER)
ALL_AVX=speed_avx_$(COMPILER) verify_avx_$(COMPILER)
ALL_AVX2=speed_avx2_$(COMPILER) verify_avx2_$(COMPILER)
ALL_AVX512=speed_avx512_$(COMPILER) verify_avx512_$(COMPILER)
ALL_ARM=speed_arm_$(COMPILER) verify_arm_$(COMPILER)
ALL_TARGETS=$(ALL) $(ALL_AVX) $(ALL_AVX2) $(ALL_AVX512)

all: $(ALL)

help:
	@echo "Intel targets:"
	@echo "x86      - makes programs verify & speed (the default target)"
	@echo "avx      - makes programs verify_avx & speed_avx"
	@echo "avx2     - makes programs verify_avx2 & speed_avx2"
	@echo "run      - runs speed test for x86 code"
	@echo "run_avx  - runs speed test for AVX code"
	@echo "run_avx2 - runs speed test for AVX2 code"
	@echo
	@echo "ARM Neon target:"
	@echo "arm      - makes programs verify_arm & speed_arm (using Neon instructions)"
	@echo "run_arm  - runs speed test for Neon code"

x86: $(ALL)

avx: $(ALL_AVX)

avx2: $(ALL_AVX2)

arm: $(ALL_ARM)

speed_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_SSE) speed.cpp -o $@

verify_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_SSE) verify.cpp -o $@

speed_avx_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_AVX) speed.cpp -o $@

verify_avx_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_AVX) verify.cpp -o $@

speed_avx2_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_AVX2) speed.cpp -o $@

verify_avx2_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_AVX2) verify.cpp -o $@

speed_avx512_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_AVX512BW) speed.cpp -o $@

verify_avx512_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_AVX512BW) verify.cpp -o $@

speed_arm_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_ARM) speed.cpp -o $@

verify_arm_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_ARM) verify.cpp -o $@

speed: speed_$(COMPILER)
speed_avx: speed_avx_$(COMPILER)
speed_avx2: speed_avx2_$(COMPILER)
speed_avx512: speed_avx512_$(COMPILER)
speed_arm: speed_arm_$(COMPILER)

verify: verify_$(COMPILER)
verify_avx: verify_avx_$(COMPILER)
verify_avx2: verify_avx2_$(COMPILER)
verify_avx512: verify_avx512_$(COMPILER)
verify_arm: verify_arm_$(COMPILER)


build-all: $(ALL_TARGETS)

SIZE=10000000
ITERS=100

run: speed
	./speed_$(COMPILER) $(SIZE) $(ITERS)

run_avx: speed_avx
	./speed_avx_$(COMPILER) $(SIZE) $(ITERS)

run_avx2: speed_avx2
	./speed_avx2_$(COMPILER) $(SIZE) $(ITERS)

run_avx512: speed_avx512
	$(SDE) -cnl -- ./speed_avx512_$(COMPILER) $(SIZE) $(ITERS)

SIZE=1000000
ITERS=100

run_arm: speed_arm
	./speed_arm_$(COMPILER) $(SIZE) $(ITERS)

run_verify: verify_$(COMPILER)
	./$^

run_verify_avx: verify_avx_$(COMPILER)
	./$^

run_verify_avx2: verify_avx2_$(COMPILER)
	./$^

run_verify_avx512: verify_avx512_$(COMPILER)
    # run via emulator
	$(SDE) -cnl -- ./$^

run_verify_arm: verify_arm_$(COMPILER)
	./$^

clean:
	rm -f $(ALL_TARGETS) $(ALL_ARM)
