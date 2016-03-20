#include <x86intrin.h>
#include <assert.h>


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

