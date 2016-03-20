// TODO: add BSD license. This code should be BSD licensed beacuse it uses
// some BSD licensed code, so making it BSD is the simplest thing.

#include <stdio.h>
#include <string.h>
#include <x86intrin.h>
#include <assert.h>
#include "../rdtsc.h"

#define MIN_LEN 256/8
#define MIN_LEN 1048576*16
#define DELTA 4
#define LINE_SIZE 128
#define ITERATIONS 10000

// Mula's SSSE3 implementation core dumps on Mac OS unless it's modified.
#define USE_SOFT

uint64_t buffer[MAX_LEN] __attribute__((aligned(LINE_SIZE)));

void setup_buffer(int len) {
  buffer[0] = 1;
  buffer[len-1] = 3;
}

// Note: this emits a popcnt with clang 3.4 but not with clang 3.0
uint32_t builtin_popcnt(const uint64_t* buf, int len) {
  int cnt = 0;
  for (int i = 0; i < len; ++i) {
    cnt += __builtin_popcountll(buf[i]);
  }
  return cnt;
}

uint32_t builtin_popcnt32(const uint64_t* buf64, int len64) {
  int cnt = 0;
  const uint32_t* buf = (const uint32_t*) buf64;
  int len = len64 * 2;
  for (int i = 0; i < len; ++i) {
    cnt += __builtin_popcount(buf[i]);
  }
  return cnt;
}

uint32_t builtin_popcnt_unrolled(const uint64_t* buf, int len) {
  assert(len % 4 == 0);
  int cnt = 0;
  for (int i = 0; i < len; i+=4) {
    cnt += __builtin_popcountll(buf[i]);
    cnt += __builtin_popcountll(buf[i+1]);
    cnt += __builtin_popcountll(buf[i+2]);
    cnt += __builtin_popcountll(buf[i+3]);
  }
  return cnt;
}

uint32_t builtin_popcnt_unrolled32(const uint64_t* buf64, int len64) {
  const uint32_t* buf = (const uint32_t*) buf64;
  int len = len64 * 2;
  assert(len % 4 == 0);
  int cnt = 0;
  for (int i = 0; i < len; i+=4) {
    cnt += __builtin_popcount(buf[i]);
    cnt += __builtin_popcount(buf[i+1]);
    cnt += __builtin_popcount(buf[i+2]);
    cnt += __builtin_popcount(buf[i+3]);
  }
  return cnt;
}

// Attempt to work around false depdency errata.
// gcc is too smart to fall for this and re-creates the dependency unless
// compiled with -funroll-loops or something similar.
// This works with clang, though.
uint32_t builtin_popcnt_unrolled_errata(const uint64_t* buf, int len) {
  assert(len % 4 == 0);
  int cnt[4];
  for (int i = 0; i < 4; ++i) {
    cnt[i] = 0;
  }

  for (int i = 0; i < len; i+=4) {
    cnt[0] += __builtin_popcountll(buf[i]);
    cnt[1] += __builtin_popcountll(buf[i+1]);
    cnt[2] += __builtin_popcountll(buf[i+2]);
    cnt[3] += __builtin_popcountll(buf[i+3]);
  }
  return cnt[0] + cnt[1] + cnt[2] + cnt[3];
}

// Here's a version that doesn't rely on the compiler not doing
// bad optimizations.
// This code is from Alex Yee.

uint32_t builtin_popcnt_unrolled_errata_manual(const uint64_t* buf, int len) {
  assert(len % 4 == 0);
  uint64_t cnt[4];
  for (int i = 0; i < 4; ++i) {
    cnt[i] = 0;
  }

  for (int i = 0; i < len; i+=4) {
    __asm__(    
	    "popcnt %4, %4  \n\t"
	    "add %4, %0     \n\t"
	    "popcnt %5, %5  \n\t"
	    "add %5, %1     \n\t"
	    "popcnt %6, %6  \n\t"
	    "add %6, %2     \n\t"
	    "popcnt %7, %7  \n\t"
	    "add %7, %3     \n\t"
	    : "+r" (cnt[0]), "+r" (cnt[1]), "+r" (cnt[2]), "+r" (cnt[3])
	    : "r"  (buf[i]), "r"  (buf[i+1]), "r"  (buf[i+2]), "r"  (buf[i+3])    
		);
  }
  return cnt[0] + cnt[1] + cnt[2] + cnt[3];
}


// This works as intended with clang, but gcc turns the MOVQ intrinsic into an xmm->mem 
// operation which defeats the purpose of using MOVQ.

uint32_t builtin_popcnt_movdq(const uint64_t* buf, int len) {
  int cnt = 0;
  __m128i temp;
  __m128i temp2;
  uint64_t lower64;
  uint64_t upper64;

  for (int i = 0; i < len; i+=2) {
    temp = _mm_load_si128((__m128i*)&buf[i]);
    lower64 = _mm_cvtsi128_si64(temp);
    cnt += __builtin_popcountll(lower64);
    temp2 = (__m128i)_mm_movehl_ps((__m128)temp, (__m128)temp);
    upper64 = _mm_cvtsi128_si64(temp2);
    cnt += __builtin_popcountll(upper64);
  }
  return cnt;
}

// With gcc, this code has the same problem as the previous fn, where movq
// gets translated into an xmm->mem movq.
// Clang handles the movq correctly but it optimizes away the seperate cnt
// variables, causing the popcnt false register dependcy to reduce performance.

uint32_t builtin_popcnt_movdq_unrolled(const uint64_t* buf, int len) {
  int cnt[4];
  __m128i temp[2];
  __m128i temp_upper[2];
  uint64_t lower64[2];
  uint64_t upper64[2];

  for (int i = 0; i < 2; ++i) {
    cnt[i] = 0;
  }

  for (int i = 0; i < len; i+=4) {
    temp[0] = _mm_load_si128((__m128i*)&buf[i]);
    temp[1] = _mm_load_si128((__m128i*)&buf[i+2]);
    lower64[0] = _mm_cvtsi128_si64(temp[0]);
    lower64[1] = _mm_cvtsi128_si64(temp[1]);
    cnt[0] += __builtin_popcountll(lower64[0]);
    cnt[1] += __builtin_popcountll(lower64[1]);
    temp_upper[0] = (__m128i)_mm_movehl_ps((__m128)temp[0], (__m128)temp[0]);
    temp_upper[1] = (__m128i)_mm_movehl_ps((__m128)temp[1], (__m128)temp[1]);
    upper64[0] = _mm_cvtsi128_si64(temp_upper[0]);
    upper64[1] = _mm_cvtsi128_si64(temp_upper[1]);
    cnt[2] += __builtin_popcountll(upper64[0]);
    cnt[3] += __builtin_popcountll(upper64[1]);
  }
  return cnt[0] + cnt[1] + cnt[2] + cnt[3];
}

uint32_t builtin_popcnt_movdq_unrolled_manual(const uint64_t* buf, int len) {
  uint64_t cnt[4];
  __m128i temp_upper[2];
  uint64_t lower64[2];
  uint64_t upper64[2];

  for (int i = 0; i < 2; ++i) {
    cnt[i] = 0;
  }

  for (int i = 0; i < len; i+=4) {
    __m128i x0 = _mm_load_si128((__m128i*)&buf[i]);
    __m128i x1 = _mm_load_si128((__m128i*)&buf[i+2]);

    __m128i x0_upper;
    __m128i x1_upper;

    uint64_t dummy0;
    uint64_t dummy1;
    uint64_t dummy0_upper;
    uint64_t dummy1_upper;

    __asm__(    	    
	    "movhlps %10, %6 \n\t"
	    "movhlps %11, %7 \n\t"
	    "movq %10, %4    \n\t"
	    "movq %11, %5    \n\t"
	    "popcnt %4, %4  \n\t"
	    "add %4, %0     \n\t"
	    "popcnt %5, %5  \n\t"
	    "add %5, %1     \n\t"
	    "movq %6, %8    \n\t"
	    "movq %7, %9    \n\t"
	    "popcnt %8, %8  \n\t"
	    "add %8, %2     \n\t"
	    "popcnt %9, %9  \n\t"
	    "add %9, %3     \n\t"
	    : "+r" (cnt[0]), "+r" (cnt[1]), "+r" (cnt[2]), "+r" (cnt[3]), 
	      "=&r" (dummy0), "=&r" (dummy1),	"=x" (x0_upper), "=x" (x1_upper), 
	      "=&r" (dummy0_upper), "=&r" (dummy1_upper)
	    : "x"  (x0), "x"  (x1)
		);
  }
  return cnt[0] + cnt[1] + cnt[2] + cnt[3];
}

int run_and_time_fn(int len, int iterations, uint32_t(*fn)(const uint64_t* buf, int)) {

  uint32_t total = 0;
  uint64_t tsc_before, tsc_after, tsc, min_tsc;
  min_tsc = 0;
  min_tsc--;

  for (int i = 0; i < iterations; ++i) {
    for (int i = 0; i < len; ++i) {
      asm volatile("" :: "m" (buffer[i]));
    }
    RDTSC_START(tsc_before);
    total += fn(buffer, len);
    RDTSC_STOP(tsc_after);
    tsc = tsc_after - tsc_before;
    min_tsc = min_tsc < tsc ? min_tsc : tsc;
  }

  //  assert(total == iterations * 3); // Check that we don't have an off by one error.

  asm volatile("" :: "m" (total));
  return min_tsc;
}

// Code from Wojciech Mula.

// lookup for SSE
uint8_t POPCOUNT_4bit[16] __attribute__((aligned(16))) = {
  /* 0 */ 0,
  /* 1 */ 1,
  /* 2 */ 1,
  /* 3 */ 2,
  /* 4 */ 1,
  /* 5 */ 2,
  /* 6 */ 2,
  /* 7 */ 3,
  /* 8 */ 1,
  /* 9 */ 2,
  /* a */ 2,
  /* b */ 3,
  /* c */ 2,
  /* d */ 3,
  /* e */ 3,
  /* f */ 4
};

// ---- SSSE3 - better alorithm, inner loop unrolled ----------------------
uint32_t ssse3_popcount3(uint8_t* buffer, int chunks16) {
static char MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};

uint32_t result;

#ifdef DEBUG
assert(chunks16 % 4 == 0);
#endif

__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (POPCOUNT_4bit));
__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (MASK_4bit));
__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : ); // xmm5 -- global accumulator

result = 0;

int k, n, i;

i = 0;
while (chunks16 > 0) {
// max(POPCOUNT_8bit) = 8, thus byte-wise addition could be done
// for floor(255/8) = 31 iterations
#define MAX (7*4)
if (chunks16 > MAX) {
k = MAX;
chunks16 -= MAX;
}
else {
k = chunks16;
chunks16 = 0;
}
#undef MAX
__asm__ volatile ("pxor %xmm4, %xmm4"); // xmm4 -- local accumulator
for (n=0; n < k; n+=4) {
#define body(index) \
__asm__ volatile( \
"movdqa  (%%eax), %%xmm0\n" \
"movdqa    %%xmm0, %%xmm1\n" \
"psrlw         $4, %%xmm1\n" \
"pand      %%xmm6, %%xmm0\n" \
"pand      %%xmm6, %%xmm1\n" \
"movdqa    %%xmm7, %%xmm2\n" \
"movdqa    %%xmm7, %%xmm3\n" \
"pshufb    %%xmm0, %%xmm2\n" \
"pshufb    %%xmm1, %%xmm3\n" \
"paddb     %%xmm2, %%xmm4\n" \
"paddb     %%xmm3, %%xmm4\n" \
: : "a" (&buffer[index]));

body(i);
body(i + 1*16);
body(i + 2*16);
body(i + 3*16);
#undef body
i += 4*16;
}

// update global accumulator (two 32-bits counters)
__asm__ volatile (
"pxor   %xmm0, %xmm0\n"
"psadbw %xmm0, %xmm4\n"
"paddd  %xmm4, %xmm5\n"
);
}

// finally add together 32-bits counters stored in global accumulator
__asm__ volatile (
"movhlps   %%xmm5, %%xmm0\n"
"paddd     %%xmm5, %%xmm0\n"
"movd      %%xmm0, %%eax\n"
: "=a" (result)
);

return result;
}

// End imported code.
int run_mula_popcnt(int len, int iterations) {
  //  uint8_t buffer[LEN*8] __attribute__((aligned(LINE_SIZE))); 

  uint32_t total = 0;
  uint64_t tsc_before, tsc_after, tsc, min_tsc;
  min_tsc = 0;
  min_tsc--;

  asm volatile("" :: "m" (buffer[0]));

  for (int i = 0; i < 1000; ++i) {
    RDTSC_START(tsc_before);
    total += ssse3_popcount3((uint8_t*)buffer, len/2);
    RDTSC_STOP(tsc_after);
    tsc = tsc_after - tsc_before;
    min_tsc = min_tsc < tsc ? min_tsc : tsc;
  }

  //  assert(total == iterations * 3); // Check that we don't have an off by 1 error.

  asm volatile("" :: "m" (total));
  return min_tsc;
}

int adjusted_iterations(int len, int iterations) {
  int adjusted =  iterations * DELTA / len;
  return adjusted > 10 ? adjusted : 10;
}

double gb_per_s(int cycles, int len) {
  double bytes = len * 8;
  return (double) 3.4 * bytes / (double) cycles;
  // this 3.4 constant depends on the chip.
}

int main() {
  for (int len = MIN_LEN; len <= MAX_LEN; len *= DELTA) {
    //    int iterations = adjusted_iterations(len, ITERATIONS);
    // printf("int %lu long long %lu\n", sizeof(int), sizeof(long long));

    int iterations = ITERATIONS;
    // printf("builtin32: %.2f\n", run_and_time_fn(len, iterations, &builtin_popcnt32));
    printf("bytes: %i\n", 8*len);
    printf("builtin: %.2f\n", gb_per_s(run_and_time_fn(len, iterations, &builtin_popcnt), len));
    printf("builtin unrolled: %.2f\n", gb_per_s(run_and_time_fn(len, iterations, &builtin_popcnt_unrolled), len));
    // printf("builtin unrolled32: %.2f\n", gb_per_s(run_and_time_fn(len, iterations, &builtin_popcnt_unrolled32));
    printf("builtin errata: %.2f\n", gb_per_s(run_and_time_fn(len, iterations, &builtin_popcnt_unrolled_errata), len));
    printf("builtin manual: %.2f\n", gb_per_s(run_and_time_fn(len, iterations, &builtin_popcnt_unrolled_errata_manual), len));
    // printf("builtin movdq: %.2f\n", gb_per_s(run_and_time_fn(len, iterations, &builtin_popcnt_movdq), len));
    // printf("builtin movdq unrolled: %.2f\n", gb_per_s(run_and_time_fn(len, iterations, &builtin_popcnt_movdq_unrolled), len));
    // printf("builtin movdq manual: %.2f\n", gb_per_s(run_and_time_fn(len, iterations, &builtin_popcnt_movdq_unrolled_manual));
    #ifdef USE_SOFT 
    printf("SSSE3: %.2f\n", gb_per_s(run_mula_popcnt(len, iterations), len));
    #endif
  }
}

