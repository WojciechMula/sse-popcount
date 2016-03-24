// direct translation of popcnt-harley-seal.cpp

namespace SSE_harley_seal {

__m128i popcount(const __m128i x)
{
  const __m128i m1  = _mm_set1_epi8(0x55);
  const __m128i m2  = _mm_set1_epi8(0x33);
  const __m128i m4  = _mm_set1_epi8(0x0F);

  const __m128i t1 = x;
  const __m128i t2 = _mm_sub_epi8(t1, _mm_srli_epi16(t1, 1) & m1);
  const __m128i t3 = _mm_add_epi8(t2 & m2, _mm_srli_epi16(t2, 2) & m2);
  const __m128i t4 = _mm_add_epi8(t3, _mm_srli_epi16(t3, 4)) & m4;

  return _mm_sad_epu8(t4, _mm_setzero_si128());
}

void CSA(__m128i& h, __m128i& l, __m128i a, __m128i b, __m128i c)
{
  const __m128i u = a ^ b;
  h = (a & b) | (u & c);
  l = u ^ c;

}

uint64_t popcnt(const __m128i* data, const uint64_t size)
{
  __m128i total     = _mm_setzero_si128();
  __m128i ones      = _mm_setzero_si128();
  __m128i twos      = _mm_setzero_si128();
  __m128i fours     = _mm_setzero_si128();
  __m128i eights    = _mm_setzero_si128();
  __m128i sixteens  = _mm_setzero_si128();
  __m128i twosA, twosB, foursA, foursB, eightsA, eightsB;

  const uint64_t limit = size - size % 16;
  uint64_t i = 0;

  for(; i < limit; i += 16)
  {
    CSA(twosA, ones, ones, data[i+0], data[i+1]);
    CSA(twosB, ones, ones, data[i+2], data[i+3]);
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones, data[i+4], data[i+5]);
    CSA(twosB, ones, ones, data[i+6], data[i+7]);
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsA,fours, fours, foursA, foursB);
    CSA(twosA, ones, ones, data[i+8], data[i+9]);
    CSA(twosB, ones, ones, data[i+10], data[i+11]);
    CSA(foursA, twos, twos, twosA, twosB);
    CSA(twosA, ones, ones, data[i+12], data[i+13]);
    CSA(twosB, ones, ones, data[i+14], data[i+15]);
    CSA(foursB, twos, twos, twosA, twosB);
    CSA(eightsB, fours, fours, foursA, foursB);
    CSA(sixteens, eights, eights, eightsA, eightsB);

    total = _mm_add_epi64(total, popcount(sixteens));
  }


  total = _mm_slli_epi64(total, 4);     // * 16
  total = _mm_add_epi64(total, _mm_slli_epi64(popcount(eights), 3)); // += 8 * ...
  total = _mm_add_epi64(total, _mm_slli_epi64(popcount(fours),  2)); // += 4 * ...
  total = _mm_add_epi64(total, _mm_slli_epi64(popcount(twos),   1)); // += 2 * ...
  total = _mm_add_epi64(total, popcount(ones));

  for(; i < size; i++)
    total += popcount(data[i]);

  return lower_qword(total) + higher_qword(total);
}

} // SSE_harley_seal

uint64_t popcnt_SSE_harley_seal(const uint8_t* data, const size_t size)
{
  uint64_t total = SSE_harley_seal::popcnt((const __m128i*) data, size / 16);

  for (size_t i = size - size % 16; i < size; i++)
    total += lookup8bit[data[i]];

  return total;
}

