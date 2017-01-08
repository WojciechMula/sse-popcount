#include <assert.h>


// Note: this emits a popcnt with clang 3.4 but not with clang 3.0
uint64_t builtin_popcnt(const uint64_t* buf, int len) {
  uint64_t cnt = 0;
  for (int i = 0; i < len; ++i) {
    cnt += __builtin_popcountll(buf[i]);
  }
  return cnt;
}

uint64_t builtin_popcnt32(const uint64_t* buf64, int len64) {
  uint64_t cnt = 0;
  const uint32_t* buf = (const uint32_t*) buf64;
  int len = len64 * 2;
  for (int i = 0; i < len; ++i) {
    cnt += __builtin_popcount(buf[i]);
  }
  return cnt;
}

uint64_t builtin_popcnt_unrolled(const uint64_t* buf, int len) {
  assert(len % 4 == 0);
  uint64_t cnt = 0;
  for (int i = 0; i < len; i+=4) {
    cnt += __builtin_popcountll(buf[i]);
    cnt += __builtin_popcountll(buf[i+1]);
    cnt += __builtin_popcountll(buf[i+2]);
    cnt += __builtin_popcountll(buf[i+3]);
  }
  return cnt;
}

uint64_t builtin_popcnt_unrolled32(const uint64_t* buf64, int len64) {
  const uint32_t* buf = (const uint32_t*) buf64;
  int len = len64 * 2;
  assert(len % 4 == 0);
  uint64_t cnt = 0;
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
uint64_t builtin_popcnt_unrolled_errata(const uint64_t* buf, int len) {
  assert(len % 4 == 0);
  uint64_t cnt[4];
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

