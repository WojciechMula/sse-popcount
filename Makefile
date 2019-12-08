# user can do CXX=g++ make
# It's more fexlible to change from command line
# The make builtin rule states CXX to be g++.
.PHONY: all help clean build-all \
        x86 avx avx2 arm \
        speed verify \
        speed_avx verify_avx \
        speed_avx2 verify_avx2 \
        speed_avx512bw verify_avx512bw \
        speed_avx512vbmi verify_avx512vbmi \
        verify_avx512vpopcnt \
        speed_arm verify_arm \
        speed_aarch64 verify_aarch64

COMPILER=$(notdir $(CXX))
FLAGS=-std=c++11 -O2 -Wall -pedantic -Wextra -Wfatal-errors
FLAGS_INTEL=$(FLAGS) -mpopcnt -fabi-version=6
FLAGS_ARM=$(FLAGS) -mfpu=neon -DHAVE_NEON_INSTRUCTIONS
# It seems that for AArch64 no extra flags are needed (NEON is always available)
FLAGS_AARCH64=$(FLAGS) -DHAVE_NEON_INSTRUCTIONS -DHAVE_AARCH64_ARCHITECTURE

FLAGS_SSE=$(FLAGS_INTEL) -mssse3 -DHAVE_SSE_INSTRUCTIONS
FLAGS_AVX=$(FLAGS_INTEL) -mavx -DHAVE_AVX_INSTRUCTIONS
FLAGS_AVX2=$(FLAGS_INTEL) -mavx2 -DHAVE_AVX2_INSTRUCTIONS
FLAGS_AVX512BW=$(FLAGS_INTEL) -mavx512bw -DHAVE_AVX512BW_INSTRUCTIONS
FLAGS_AVX512VBMI=$(FLAGS_INTEL) -mavx512vbmi -DHAVE_AVX512BW_INSTRUCTIONS -DHAVE_AVX512VBMI_INSTRUCTIONS
FLAGS_AVX512VPOPCNT=$(FLAGS_INTEL) -mavx512bw -mavx512vpopcntdq -DHAVE_AVX512VPOPCNT_INSTRUCTIONS

DEPS=popcnt-*.cpp function_registry.cpp sse_operators.cpp config.h
ALL=speed_$(COMPILER) verify_$(COMPILER)
ALL_AVX=speed_avx_$(COMPILER) verify_avx_$(COMPILER)
ALL_AVX2=speed_avx2_$(COMPILER) verify_avx2_$(COMPILER)
ALL_AVX512BW=speed_avx512bw_$(COMPILER) verify_avx512bw_$(COMPILER)
ALL_AVX512VBMI=speed_avx512vbmi_$(COMPILER) verify_avx512vbmi_$(COMPILER)
ALL_AVX512VPOPCNT=verify_avx512vpopcnt_$(COMPILER)
ALL_ARM=speed_arm_$(COMPILER) verify_arm_$(COMPILER)
ALL_AARCH64=speed_aarch64_$(COMPILER) verify_aarch64_$(COMPILER)
ALL_TARGETS=$(ALL) $(ALL_AVX) $(ALL_AVX2) $(ALL_AVX512) $(ALL_AVX512BW) $(ALL_AVX512VPOPCNT)

all: $(ALL)

help:
	@echo "Intel targets:"
	@echo "x86                   - makes programs verify & speed (the default target)"
	@echo "run                   - runs benchmark program"
	@echo "run_verify            - runs verification program"
	@echo
	@echo "avx                   - makes programs verify_avx & speed_avx"
	@echo "run_avx               - runs benchmark program"
	@echo "run_verify_avx        - runs verification program"
	@echo
	@echo "avx2                  - makes programs verify_avx2 & speed_avx2"
	@echo "run_avx2              - runs benchmark program"
	@echo "run_verify_avx2       - runs verification program"
	@echo
	@echo "avx512bw              - makes programs verify_avx512bw & speed_avx512bw"
	@echo "run_avx512bw          - runs benchmark program"
	@echo "run_verify_avx512bw   - runs verification program"
	@echo
	@echo "avx512vbmi            - makes programs verify_avx512vbmi & speed_avx512vbmi"
	@echo "run_avx512vbmi        - runs benchmark program"
	@echo "run_verify_avx512vbmi - runs verification program"
	@echo
	@echo "ARM Neon target:"
	@echo "arm                   - makes programs verify_arm & speed_arm (using Neon instructions)"
	@echo "run_arm               - runs benchmark program"
	@echo "run_verify_arm        - runs verification program"

x86: $(ALL)

avx: $(ALL_AVX)

avx2: $(ALL_AVX2)

arm: $(ALL_ARM)

aarch64: $(ALL_AARCH64)

avx512bw: $(ALL_AVX512BW)

avx512vbmi: $(ALL_AVX512VBMI)

avx512vpopcnt: $(ALL_AVX512VPOPCNT)

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

speed_avx512bw_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_AVX512BW) speed.cpp -o $@

verify_avx512bw_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_AVX512BW) verify.cpp -o $@

speed_avx512vbmi_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_AVX512VBMI) speed.cpp -o $@

verify_avx512vbmi_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_AVX512VBMI) verify.cpp -o $@

verify_avx512vpopcnt_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_AVX512VPOPCNT) verify.cpp -o $@

speed_arm_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_ARM) speed.cpp -o $@

verify_arm_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_ARM) verify.cpp -o $@

speed_aarch64_$(COMPILER): $(DEPS) speed.cpp
	$(CXX) $(FLAGS_AARCH64) speed.cpp -o $@

verify_aarch64_$(COMPILER): $(DEPS) verify.cpp
	$(CXX) $(FLAGS_AARCH64) verify.cpp -o $@

speed: speed_$(COMPILER)
speed_avx: speed_avx_$(COMPILER)
speed_avx2: speed_avx2_$(COMPILER)
speed_avx512bw: speed_avx512bw_$(COMPILER)
speed_avx512vbmi: speed_avx512vbmi_$(COMPILER)
speed_arm: speed_arm_$(COMPILER)
speed_aarch64: speed_aarch64_$(COMPILER)

verify: verify_$(COMPILER)
verify_avx: verify_avx_$(COMPILER)
verify_avx2: verify_avx2_$(COMPILER)
verify_avx512bw: verify_avx512bw_$(COMPILER)
verify_avx512vbmi: verify_avx512vbmi_$(COMPILER)
verify_arm: verify_arm_$(COMPILER)
verify_aarch64: verify_aarch64_$(COMPILER)


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
	./speed_avx512bw_$(COMPILER) $(SIZE) $(ITERS)

SIZE=1000000
ITERS=100

run_arm: speed_arm
	./speed_arm_$(COMPILER) $(SIZE) $(ITERS)

run_aarch64: speed_aarch64
	./speed_aarch64_$(COMPILER) $(SIZE) $(ITERS)

run_verify: verify_$(COMPILER)
	./$^

run_verify_avx: verify_avx_$(COMPILER)
	./$^

run_verify_avx2: verify_avx2_$(COMPILER)
	./$^

run_verify_avx512bw: verify_avx512bw_$(COMPILER)
	./$^

run_verify_avx512vbmi: verify_avx512vbmi_$(COMPILER)
	./$^

run_verify_arm: verify_arm_$(COMPILER)
	./$^

clean:
	rm -f $(ALL_TARGETS) $(ALL_ARM)
