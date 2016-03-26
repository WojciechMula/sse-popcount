namespace AVX512_harley_seal {

__m512i popcount(const __m512i v)
{
  const __m512i m1 = _mm512_set1_epi8(0x55);
  const __m512i m2 = _mm512_set1_epi8(0x33);
  const __m512i m4 = _mm512_set1_epi8(0x0F);

  const __m512i t1 = _mm512_sub_epi8(v,       (_mm512_srli_epi16(v,  1) & m1));
  const __m512i t2 = _mm512_add_epi8(t1 & m2, (_mm512_srli_epi16(t1, 2) & m2));
  const __m512i t3 = _mm512_add_epi8(t2, _mm512_srli_epi16(t2, 4)) & m4;
  return _mm512_sad_epu8(t3, _mm512_setzero_si512());
}

void CSA(__m512i& h, __m512i& l, __m512i a, __m512i b, __m512i c)
{
    /*
        c b a | l h
        ------+----
        0 0 0 | 0 0
        0 0 1 | 1 0
        0 1 0 | 1 0
        0 1 1 | 0 1
        1 0 0 | 1 0
        1 0 1 | 0 1
        1 1 0 | 0 1
        1 1 1 | 1 1

        l - digit
        h - carry
    */

  l = _mm512_ternarylogic_epi32(c, b, a, 0x96);
  h = _mm512_ternarylogic_epi32(c, b, a, 0xe8);
}

uint64_t popcnt(const __m512i* data, const uint64_t size)
{
  __m512i total     = _mm512_setzero_si512();
  __m512i ones      = _mm512_setzero_si512();
  __m512i twos      = _mm512_setzero_si512();
  __m512i fours     = _mm512_setzero_si512();
  __m512i eights    = _mm512_setzero_si512();
  __m512i sixteens  = _mm512_setzero_si512();
  __m512i twosA, twosB, foursA, foursB, eightsA, eightsB;

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

    total = _mm512_add_epi64(total, popcount(sixteens));
  }

  total = _mm512_slli_epi64(total, 4);     // * 16
  total = _mm512_add_epi64(total, _mm512_slli_epi64(popcount(eights), 3)); // += 8 * ...
  total = _mm512_add_epi64(total, _mm512_slli_epi64(popcount(fours),  2)); // += 4 * ...
  total = _mm512_add_epi64(total, _mm512_slli_epi64(popcount(twos),   1)); // += 2 * ...
  total = _mm512_add_epi64(total, popcount(ones));

  for(; i < size; i++)
    total = _mm512_add_epi64(total, popcount(data[i]));


  return simd_sum_epu64(total);
}

} // AVX512_harley_seal

uint64_t popcnt_AVX512_harley_seal(const uint8_t* data, const size_t size)
{
  uint64_t total = AVX512_harley_seal::popcnt((const __m512i*) data, size / 64);

  for (size_t i = size - size % 64; i < size; i++)
    total += lookup8bit[data[i]];

  return total;
}

