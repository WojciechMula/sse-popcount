bin_PROGRAMS = verify speed
verify_SOURCES = verify.cpp config.h
verify_CXXFLAGS = @X86_FEATURE_CFLAGS@
speed_SOURCES = speed.cpp config.h
speed_CXXFLAGS = @X86_FEATURE_CFLAGS@

SIZE=10000000
ITERS=1000

run: speed
	./speed $(SIZE) $(ITERS)
